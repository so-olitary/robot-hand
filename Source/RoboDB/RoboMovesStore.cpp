﻿#include "RoboMovesStore.h"

//------------------------------------------------------------------------------
size_t RoboMoves::Store::adjacencyByXYBorders(IN  const Point &aim, IN double side,
                                              OUT std::pair<Record, Record> &x_pair,
                                              OUT std::pair<Record, Record> &y_pair) const
{
    boost::lock_guard<boost::mutex> lock(_store_mutex);
    //-------------------------------------------------------
    size_t count = 0U;
    //-------------------------------------------------------
    {
        const MultiIndexMoves::index<ByX>::type &X_index = _store.get<ByX>();
        auto pair_x = X_index.range((aim.x - side) <= boost::lambda::_1, boost::lambda::_1 <= (aim.x + side));

        decltype(pair_x.first)  it_max_x = X_index.end();
        decltype(pair_x.first)  it_min_x = X_index.end();
        for (auto it = pair_x.first; it != pair_x.second; ++it)
        {
            if ((it->hit.x < aim.x) && (it_max_x == X_index.end() || it_max_x->hit.x < it->hit.x))
            { it_max_x = it; }

            if ((it->hit.x > aim.x) && (it_min_x == X_index.end() || it_min_x->hit.x > it->hit.x))
            { it_min_x = it; }
        }

        if (it_max_x != X_index.end() && it_min_x != X_index.end())
        { x_pair = std::make_pair(*it_max_x, *it_min_x); count += 2U; }
    }
    //-------------------------------------------------------
    {
        const MultiIndexMoves::index<ByY>::type  &Y_index = _store.get<ByY>();
        auto pair_y = Y_index.range((aim.y - side) <= boost::lambda::_1, boost::lambda::_1 <= (aim.y + side));

        decltype(pair_y.first)  it_max_y = Y_index.end();
        decltype(pair_y.first)  it_min_y = Y_index.end();
        for (auto it = pair_y.first; it != pair_y.second; ++it)
        {
            if ((it->hit.y < aim.y) && (it_max_y == Y_index.end() || it_max_y->hit.y < it->hit.y))
            { it_max_y = it; }

            if ((it->hit.y > aim.y) && (it_min_y == Y_index.end() || it_min_y->hit.y > it->hit.y))
            { it_min_y = it; }
        }

        if (it_max_y != Y_index.end() && it_min_y != Y_index.end())
        { y_pair = std::make_pair(*it_max_y, *it_min_y); count += 2U; }
    }
    //-------------------------------------------------------
    return count;
}

// --------------------------------------------------------------
void RoboMoves::Store::draw(HDC hdc, double radius, const GetHPen &getPen) const
{
#ifdef MY_WINDOW
    try
    {
        unsigned i = 0;
        for (const auto &rec : _store)
        {
            drawCircle(hdc, rec.hit, radius, getPen(rec));
            if (++i == 500)
            {
                boost::this_thread::interruption_point();
                i = 1;
            }
        }
    }
    catch (boost::thread_interrupted&)
    { CINFO("WorkingThread interrupted"); }
#endif // MY_WINDOW
}

//------------------------------------------------------------------------------
void RoboMoves::Store::insert(const Record &rec)
{
    try
    {
        /* Массив траекторий для каждой точки.
        *  Несколько вариантов одного и того же движения.
        *  Вверх класть более удачные движения, отн. кол-ва действий и точности попадания.
        */
        boost::this_thread::disable_interruption no_interruption;
        boost::lock_guard<boost::mutex> lock(_store_mutex);
        // ==============================
        auto status = _store.insert(rec);
        status.first->updateTraj(_trajectories_enumerate);
        // ==============================
        if (status.second)
            for (const auto &p : rec.trajectory)
                _inverse.push_back({ &p, &(*status.first) });
    }
    catch (const std::exception &e)
    {
        SHOW_CERROR(e.what());
    }
}

//------------------------------------------------------------------------------
void RoboMoves::Store::dump_off(const tstring &filename, const Robo::RoboI &robo, Format format) const
{
    try
    {
        boost::this_thread::disable_interruption no_interruption;
        boost::lock_guard<boost::mutex> lock(_store_mutex);

        // --- header ---
        tptree root;
        robo.save(root);
        // --------------
        if (format == Format::TXT)
        {
            std::ofstream ofs(filename, std::ios_base::out);
            boost::archive::text_oarchive toa(ofs);
            toa << root << this;
        }
        else if (format == Format::BIN)
        {
            std::ofstream ofs(filename, std::ios_base::out | std::ios_base::binary);
            boost::archive::binary_oarchive boa(ofs);
            boa << root << *this;
        }
        else if (format != Format::NONE)
            throw std::runtime_error("Invalid store format");
        CINFO("saved '" << filename << "' store " << size() << " inverse " << _inverse.size());
    }
    catch (const std::exception &e)
    {
        SHOW_CERROR(e.what());
    }
}

//------------------------------------------------------------------------------
void RoboMoves::Store::pick_up(const tstring &filename, std::shared_ptr<Robo::RoboI> &pRobo, Format format)
{
    if (!bfs::exists(filename))
        CERROR(_T("File '") << filename << _T("' does not exists."));

    clear();

    try
    {
        boost::this_thread::disable_interruption no_interruption;
        boost::lock_guard<boost::mutex> lock(_store_mutex);

        tptree root;
        if (format == Format::TXT)
        {
            std::ifstream ifs(filename, std::ios_base::in);
            boost::archive::text_iarchive tia(ifs);
            tia >> root >> *this;
        }
        else if (format == Format::BIN)
        {
            std::ifstream ifs(filename, std::ios_base::in | std::ios_base::binary);
            boost::archive::binary_iarchive bia(ifs);
            bia >> root >> *this;
        }
        else if (format != Format::NONE)
            throw std::runtime_error("Invalid store format");

        for (const auto &rec : _store)
            for (const auto &p : rec.trajectory)
                _inverse.push_back({ &p, &rec });

        CINFO("loaded '" << filename << "' store " << size() << " inverse " << _inverse.size());
        // --- header ---
        Factory<Robo::RoboI> frobo;
        auto pNewRobo = frobo.create(root);
        if (*pNewRobo != *pRobo)
        {
            CINFO("change robo from " << pRobo->getName() << " to " << pNewRobo->getName());
            pRobo = pNewRobo;
        }
        // --------------
    }
    catch (const std::exception &e)
    {
        SHOW_CERROR(e.what());
    }
}

//------------------------------------------------------------------------------
bool RoboMoves::Store::near_passed_build_index()
{
    if (_inverse_index_last == _inverse.size())
        return true;

    try
    {
        boost::lock_guard<boost::mutex> lock(_store_mutex);

        _inverse_kdtree.addPoints(_inverse_index_last, _inverse.size() - 1);
        CINFO("built " << _inverse.size() - _inverse_index_last);
        _inverse_index_last = _inverse.size();
        return true;
    }
    catch (const std::exception &e)
    {
        SHOW_CERROR(e.what());
        return false;
    }
}

