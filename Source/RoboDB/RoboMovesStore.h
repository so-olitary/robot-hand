﻿#pragma once

// https://github.com/jlblancoc/nanoflann
#include "nanoflann.hpp"

#include "RoboMovesRecord.h"
#ifdef MY_WINDOW
#include "WindowHeader.h"
#include "WindowDraw.h"
#endif // MY_WINDOW

namespace RoboPos {
class Approx;
}

namespace RoboMoves
{
  struct ByX {}; ///< by x of hit
  struct ByY {}; ///< by y of hit
  struct ByP {}; ///< by x and y of hit
  struct ByA {}; ///< by x and y of aim
  struct ByC {}; ///< by controls
  struct ByL {}; ///< by trajectory length
  //struct ByD {}; ///< by distance

  typedef std::list<Record>                   adjacency_t;
  typedef std::list<const Record*>            adjacency_ptrs_t;
  typedef std::list<std::shared_ptr<Record>>  adjacency_sh_ptrs_t;

  using ApproxFilter = std::function<const Record*()>;
  //------------------------------------------------------------------------------
  class  ClosestPredicate
  {
      Point _aim;
  public:
    ClosestPredicate(const Point &aim) : _aim(aim) {}
    double operator()(const std::shared_ptr<Record> &a,
                      const std::shared_ptr<Record> &b)
    { return this->operator()(a->hit, b->hit); }
    double operator()(const Record *a, const Record *b)
    { return this->operator()(a->hit, b->hit); }
    double operator()(const Record &a, const Record &b)
    { return this->operator()(a.hit, b.hit); }
    double operator()(const Point &a, const Point &b)
    { return (bg::distance(_aim, a) < bg::distance(_aim, b)); }
  };
  //------------------------------------------------------------------------------
  class  ControlHasher //: std::unary_function<const Robo::Control&, size_t>
  {
  public:
    std::size_t operator()(const Robo::Control &controls) const
    {
      std::size_t seed = 0U;
      for (auto& c : controls)
      {
        boost::hash_combine(seed, boost::hash_value (c.muscle));
        boost::hash_combine(seed, boost::hash_value (c.start));
        boost::hash_combine(seed, boost::hash_value (c.lasts));
      }
      return seed;
    }
  };
  //------------------------------------------------------------------------------
  class  RangeInserter
  {
  public:
    void operator()(OUT adjacency_t &range, IN const Record &rec)
    { range.push_back(rec); }
    void operator()(OUT adjacency_ptrs_t &range, IN const Record &rec)
    { range.push_back(/* (Record*) */(&rec)); }
    void operator()(OUT adjacency_sh_ptrs_t &range, IN const Record &rec)
    { range.push_back(std::make_shared<Record>(rec)); }
  };
  //------------------------------------------------------------------------------
  using namespace boost::multi_index;
  /// Robot Moves DataBase
  class Store
  {
    //------------------------------------------------------------------------------
    using MultiIndexMoves = boost::multi_index_container
    < Record,
      indexed_by < hashed_unique      < tag<ByC>,
                                        const_mem_fun < Record, const Robo::Control&, &Record::getControls>,
                                        ControlHasher
                                      >,
                   ordered_non_unique < tag<ByP>,
                                        composite_key < Record,
                                                        const_mem_fun<Record, Robo::distance_t, &Record::hit_x >,
                                                        const_mem_fun<Record, Robo::distance_t, &Record::hit_y >
                                                      >
                                      >,
                   //ordered_non_unique < tag<ByA>,
                   //                     composite_key < Record,
                   //                                     const_mem_fun<Record, Robo::distance_t, &Record::aim_x >,
                   //                                     const_mem_fun<Record, Robo::distance_t, &Record::aim_y >
                   //                                   >
                   //                   >,
                   //ordered_non_unique < tag<ByL>, const_mem_fun<Record, Robo::distance_t, &Record::distanceCovered > >,
                   ordered_non_unique < tag<ByX>, const_mem_fun<Record, Robo::distance_t, &Record::hit_x > >,
                   ordered_non_unique < tag<ByY>, const_mem_fun<Record, Robo::distance_t, &Record::hit_y > >
                   // , random_access      < > // доступ, как у вектору
                 >
    >;

    using InverseIndex = std::vector<std::pair<const Point*, const Record*>>;
    //==============================================================================   
    MultiIndexMoves _store{};
    mutable boost::mutex _store_mutex{};
    InverseIndex _inverse{};
    size_t _inverse_index_last{ 0 };
    /// интерполяция функции(x,y) остановки по управлениям мускулов
    std::shared_ptr<RoboPos::Approx> _approx{};

    //==============================================================================
    SimTime _trajectories_enumerate{ 0 };
    //==============================================================================
    /// \returns number of points in kdtree
    inline size_t kdtree_get_point_count() const { return _inverse.size(); }
    
    /// \returns the dim-th component of the i-th point
    inline Robo::distance_t kdtree_get_pt(const size_t i, int dim) const
    { return (*_inverse[i].first)[i]; }
        
    /// Optional bounding-box computation: return false by default
    /// \return true if the BBOX was already computed (to avoid the redoing) and return it in "bb"
    template <class BBOX>
    bool kdtree_get_bbox(BBOX&/*bb*/) const { return false; }
    
    static const size_t KDTDim = 2;
    using KDTDist = nanoflann::L2_Simple_Adaptor<Robo::distance_t, Store>;
    using KDTree = nanoflann::KDTreeSingleIndexDynamicAdaptor<KDTDist, Store, KDTDim>;    
    using KDTree1 = nanoflann::KDTreeSingleIndexDynamicAdaptor_<KDTDist, Store, KDTDim, size_t>;
    using KDTreeBase = nanoflann::KDTreeBaseClass<KDTree1, KDTDist, Store, KDTDim, size_t>;
    friend class KDTreeBase;
    friend class KDTree1;
    friend class KDTree;
    //==============================================================================
    /// Construct a kd-tree index
    KDTree _inverse_kdtree{ KDTDim, *this, nanoflann::KDTreeSingleIndexAdaptorParams(10) };
    //==============================================================================
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive &ar, unsigned version)
    { ar & _store; }

  public:
    enum class Format { NONE = 0, TXT = 1, BIN = (1 << 1) };

    Store() = default;
    Store(Store&&) = delete;
    Store(const Store&) = delete;
    Store(const tstring &filename, std::shared_ptr<Robo::RoboI> &r, Format f) : Store() { pick_up(filename, r, f); }
    //------------------------------------------------------------------------------
    /// \return the closest hit in a square adjacency of the aim point
    std::pair<bool, Record> getClosestPoint(IN const Point &aim, IN double side) const;
    Record closestEndPoint(const Point &aim) const;
    //------------------------------------------------------------------------------
    using Mod = std::function<void(Record&)>;
    void replace(IN const Robo::Control &controls, IN const Mod &mod);
    //------------------------------------------------------------------------------
    /* прямоугольная окрестность точки */
    template <class range_t, class index_t>
    size_t  adjacencyRectPoints(OUT range_t &range, IN const Point &min, IN const Point &max) const
    {
      boost::lock_guard<boost::mutex> lock(_store_mutex);
      // -----------------------------------------------
      static_assert ( boost::is_same<range_t, adjacency_t>::value
                   || boost::is_same<range_t, adjacency_ptrs_t>::value
                   || boost::is_same<range_t, adjacency_sh_ptrs_t>::value,
                      "Incorrect type to template function." );
      // -----------------------------------------------
      typedef MultiIndexMoves::index<index_t>::type::const_iterator Index_cIter;
      const MultiIndexMoves::index<index_t>::type &index = _store.get<index_t> ();   
      // -----------------------------------------------
      /* Range searching, i.e.the lookup of all elements in a given interval */
      Index_cIter itFirstLower = index.lower_bound (boost::tuple<double, double> (min));
      Index_cIter itFirstUpper = index.upper_bound (boost::tuple<double, double> (max));
      // -----------------------------------------------
      RangeInserter rangeInserter;
      // -----------------------------------------------
      for ( auto it = itFirstLower; it != itFirstUpper; ++it )
      {
        if ( (min.x <= it->hit.x && min.y <= it->hit.y)
          && (max.x >= it->hit.x && max.y >= it->hit.y) )
            rangeInserter(range, *it);
      }
      CINFO(" min=" << min << " max=" << max << " r=" << bg::distance(min, max)/2 << " adjacency=" << range.size());
      // -----------------------------------------------
      return range.size();
    }
    /* круглая окрестность точки */
    template <class range_t>
    size_t adjacencyPoints(OUT range_t &range, IN const Point &aim, IN double radius) const
    {
        boost::lock_guard<boost::mutex> lock(_store_mutex);
        // -----------------------------------------------
        static_assert (boost::is_same<range_t, adjacency_t>::value
                       || boost::is_same<range_t, adjacency_ptrs_t>::value
                       || boost::is_same<range_t, adjacency_sh_ptrs_t>::value,
                       "Incorrect type to template function.");
        // -----------------------------------------------
        typedef MultiIndexMoves::index<ByP>::type::const_iterator IndexPcIter;
        const MultiIndexMoves::index<ByP>::type &index = _store.get<ByP>();
        // -----------------------------------------------
        IndexPcIter itFirstLower = index.lower_bound(boost::make_tuple(aim.x - radius, aim.y - radius));
        IndexPcIter itFirstUpper = index.upper_bound(boost::make_tuple(aim.x + radius, aim.y + radius));
        // -----------------------------------------------
        RangeInserter rangeInserter;
        // -----------------------------------------------
        for (auto it = itFirstLower; it != itFirstUpper; ++it)
            if (boost_distance(aim, it->hit) <= radius)
                rangeInserter(range, *it);

        CINFO(" aim=" << aim << " r=" << radius << " adj_n_points=" << range.size());
        // -----------------------------------------------
        return range.size();
    }
    //------------------------------------------------------------------------------
    template <class range_t>
    size_t similDistances(OUT range_t &range, IN double min_distance, IN double max_distance) const
    {
        boost::lock_guard<boost::mutex> lock(_store_mutex);
        // -----------------------------------------------
        static_assert (boost::is_same<range_t, adjacency_t>::value
                       || boost::is_same<range_t, adjacency_ptrs_t>::value
                       || boost::is_same<range_t, adjacency_sh_ptrs_t>::value,
                       "Incorrect type to template function.");
        // -----------------------------------------------
        typedef MultiIndexMoves::index<ByD>::type::const_iterator IndexDcIter;
        MultiIndexMoves::index<ByD>::type  &index = _store.get<ByD>();
        // -----------------------------------------------
        IndexDcIter itFirstLower = index.lower_bound(min_distance);
        IndexDcIter itFirstUpper = index.upper_bound(max_distance);
        // -----------------------------------------------
        RangeInserter  rangeInserter;
        // -----------------------------------------------
        for (auto it = itFirstLower; it != itFirstUpper; ++it)
        { rangeInserter(range, *it); }
        // -----------------------------------------------
        return range.size();
    }
    //------------------------------------------------------------------------------
    template <class range_t>
    size_t adjacencyByPBorders(OUT range_t &range, IN const Point &aim, IN double side) const
    {
        adjacencyRectPoints<range_t, ByP>(range, Point(aim.x - side, aim.y - side),
                                                 Point(aim.x + side, aim.y + side));
        // -----------------------------------------------
        ClosestPredicate cp(aim);
        range.sort(cp);
        // -----------------------------------------------
        int i = 0, j = 0, k = 0, l = 0;
        // -----------------------------------------------
        auto it = range.begin();
        while (it != range.end())
        {
            if (((**it).hit.x < aim.x && (**it).hit.y < aim.y && !i))
            { ++i; ++it; }
            else if (((**it).hit.x < aim.x && (**it).hit.y > aim.y && !j))
            { ++j; ++it; }
            else if (((**it).hit.x > aim.x && (**it).hit.y < aim.y && !k))
            { ++k; ++it; }
            else if (((**it).hit.x > aim.x && (**it).hit.y > aim.y && !l))
            { ++l; ++it; }
            else
            { it = range.erase(it); }
        }
        // -----------------------------------------------
        return range.size();
    }
    //------------------------------------------------------------------------------
    size_t adjacencyByXYBorders(IN  const Point &aim, IN double side,
                                OUT std::pair<Record, Record> &x_pair,
                                OUT std::pair<Record, Record> &y_pair) const;
    //------------------------------------------------------------------------------
    const Record* exactRecordByControl(IN const Robo::Control &controls) const;
    //------------------------------------------------------------------------------
    template <class range_t>
    size_t findEndPoint(OUT range_t &range, IN const Point &aim) const
    {
        boost::lock_guard<boost::mutex>  lock(_store_mutex);
        // -----------------------------------------------
        static_assert (boost::is_same<range_t, adjacency_t>::value
                       || boost::is_same<range_t, adjacency_ptrs_t>::value
                       || boost::is_same<range_t, adjacency_sh_ptrs_t>::value,
                       "Incorrect type to template function.");
        // -----------------------------------------------
        MultiIndexMoves::index<ByP>::type  &index = _store.get<ByP>();
        auto result = index.equal_range(boost::tuple<double, double>(aim));
        // -----------------------------------------------
        size_t count = 0U;
        RangeInserter rangeInserter;
        for (auto it = result.first; it != result.second(); ++it)
        {
            rangeInserter(range, *it);
            ++count;
        }
        CINFO(" aim " << aim << " count " << count);
        // -----------------------------------------------
        return count;
    }
    //------------------------------------------------------------------------------
    using GetHPen = std::function<HPEN(const RoboMoves::Record&)>;
    void draw(HDC hdc, double radius) const
    {
        auto getPen = [](const Record&) { return (HPEN)GetStockObject(BLACK_PEN); };
        draw(hdc, radius, getPen);
    }
    void draw(HDC hdc, double radius, HPEN hPen) const
    {
        auto getPen = [hPen](const Record&) { return hPen; };
        draw(hdc, radius, getPen);
    }
    void draw(HDC hdc, double radius, const GetHPen&) const;
    //------------------------------------------------------------------------------
    void dump_off(const tstring &filename, const Robo::RoboI &robo, Format format) const;
    void pick_up(const tstring &filename, std::shared_ptr<Robo::RoboI> &robo, Format format);
    //------------------------------------------------------------------------------
    template <class range_t>
    size_t near_passed_points(OUT range_t &range, IN const Point &p, IN Robo::distance_t radius) const
    {
        static_assert (boost::is_same<range_t, adjacency_t>::value
                    || boost::is_same<range_t, adjacency_ptrs_t>::value
                    || boost::is_same<range_t, adjacency_sh_ptrs_t>::value,
                       "Incorrect type to template function.");
        // -----------------------------------------------
        near_passed_build_index();
        // -----------------------------------------------
        {
            boost::lock_guard<boost::mutex> lock(_store_mutex);
            // -----------------------------------------------
            double aim[] = { p.x, p.y };
            nanoflann::SearchParams searchParams{ 10 /* !IGNORED */, Utils::EPSILONT, true /* sorted by distance to aim */ };

            using inverse_pos_t = size_t;
            std::vector<std::pair<inverse_pos_t, distance_t>> found_points_positions; // result
            size_t n_found = _inverse_kdtree.radiusSearch(aim, radius, found_points_positions, searchParams);
            // -----------------------------------------------
            RangeInserter rangeInserter;
            for (auto &p : found_points_indices)
                rangeInserter(range, _inverse[p.first].second);
        }
        CDEBUG(" near to " << p << " passed " << n_found);
        return n_found;
    }
    /// Construct Inverse Index of all passed points
    void near_passed_build_index();
    //------------------------------------------------------------------------------
    void construct_approx(size_t max_n_controls, ApproxFilter &filter);
    RoboPos::Approx* approx();
    //------------------------------------------------------------------------------
    using MultiIndexMovesIxPcIter = MultiIndexMoves::index<ByP>::type::const_iterator;
    using MultiIndexMovesSqPassing = std::pair<MultiIndexMovesIxPcIter, MultiIndexMovesIxPcIter>;
    /// iterators-pair using: for (auto it=ret.first; it!=ret.second; ++it) {}
    /// \return all points in a square adjacency for the aim point
    MultiIndexMovesSqPassing aim_sq_adjacency(IN const Point &aim, IN double side) const
    {
        //boost::lock_guard<boost::mutex> lock(_store_mutex);
        const auto &indexP = _store.get<ByP>();
        // -----------------------------------------------
        auto itPL = indexP.lower_bound(boost::make_tuple(aim.x - side, aim.y - side));
        auto itPU = indexP.upper_bound(boost::make_tuple(aim.x + side, aim.y + side));
        // -----------------------------------------------
        CDEBUG(" aim " << aim << " side " << side << " adjacency");
        return std::make_pair(itPL, itPU);
    }
    MultiIndexMovesSqPassing aim_sq_adjacency(IN const Point &aim, IN const Point &min, IN const Point &max) const
    {
        //boost::lock_guard<boost::mutex> lock(_store_mutex);
        const auto &index = _store.get<ByP>();
        // -----------------------------------------------
        auto itPL = index.lower_bound(boost::tuple<double, double>(min));
        auto itPU = index.upper_bound(boost::tuple<double, double>(max));
        // -----------------------------------------------
        CDEBUG(" min=" << min << " max=" << max << " r=" << bg::distance(min, max) / 2 << " adjacency" /*<< (itPU - itPL)*/);
        return std::make_pair(itPL, itPU);
    }
    // // get all points in round adjacency for the aim point
    //using MultiIndexMovesRnPassing = std::pair<MultiIndexMovesIxDcIter, MultiIndexMovesIxDcIter>;
    //MultiIndexMovesRnPassing aim_rn_adjacency(IN const Point &aim, IN double radius) const
    //{
    //    //boost::lock_guard<boost::mutex> lock(_store_mutex);
    //    //MultiIndexMoves::index<ByD>::type
    //    const auto &indexD = _store.get<ByD>();
    //    CINFO(" aim " << aim << " r " << radius << " adjacency");
    //    return std::make_pair(indexD.lower_bound(0), indexD.upper_bound(radius));
    //}
    //------------------------------------------------------------------------------
    void  insert(const Record &rec);
    //------------------------------------------------------------------------------
    auto  begin()       -> decltype(_store.begin()) { return _store.begin (); }
    auto  begin() const -> decltype(_store.begin()) { return _store.begin (); }

    auto  end()       -> decltype(_store.end()) { return _store.end (); }
    auto  end() const -> decltype(_store.end()) { return _store.end (); }
    //------------------------------------------------------------------------------
    void  clear();
    bool  empty() const
    {
        // boost::lock_guard<boost::mutex>  lock(_store_mutex);
        return _store.empty();
    }
    size_t size() const
    {
        // boost::lock_guard<boost::mutex>  lock(_store_mutex);
        return _store.size();
    }
  };
}
//------------------------------------------------------------------------------
BOOST_CLASS_VERSION(RoboMoves::Store, 2)
//------------------------------------------------------------------------------
