﻿#include "StdAfx.h"

#ifndef  _POSITION_H_
#define  _POSITION_H_
//------------------------------------------------------------------------------
#define HAND_VER 2
#if   HAND_VER == 1
#include "Hand.h"
using namespace OldHand;
#elif HAND_VER == 2
#include "NewHand.h"
using namespace NewHand;
#include "HandMuscles.h"
#endif

//------------------------------------------------------------------------------
#include "MyWindow.h"
#include "target.h"

namespace Positions
{
#define _HAND_TEST_CONSOLE_PRINTF

  //void  /*HandMoves::*/testCover (Store &store, Hand &hand,
  //                            Hand::MusclesEnum muscles /* recursive */,
  //                            int recursive,
  //                            std::list<int> step,
  //                            std::list<Point> trajectory1)
  //{
  //  // hand.reset ();
  //  hand.SET_DEFAULT;
  //
  //  Point hand_base = hand.position;
  //
  //  for ( int i = step[recursive]; i < hand.muscles_.size (); ++i )
  //  {
  //    auto muscle_i = hand.muscles_[i];
  //    if ( !(muscle_i & muscles) )
  //    {
  //      trajectory_t  trajectory;
  //
  //      /* Попробуем его варьировать по длительности */
  //      for ( Hand::frames_t last_i : boost::irange (1U, hand.maxMuscleLast (muscle_i)) )
  //      {
  //        hand.move (muscle_i, last_i, trajectory);
  //        store.insert (Record (hand.position, hand_base, hand.position,
  //                             { muscle_i }, { 0 }, { last_i }, 1U,
  //                             trajectory)
  //                     );
  //
  //        std::copy (trajectory.begin (), trajectory.end (), trajectory1.begin ());
  //        if ( step.size < recursive + 1 )
  //        {}
  //
  //        testCover (store, hand, muscles & muscle_i, recursive + 1, step, trajectory1);
  //
  //      } // end for
  //    } // end if
  //  } // end for
  //}

  // using namespace std;
  // using namespace HandMoves;
  //------------------------------------------------------------------------------
  static void  testCover (HandMoves::Store &store, Hand &hand, HandMoves::trajectories_t &trajectories)
  {
    hand.reset ();
    hand.SET_DEFAULT;

    Point hand_base = hand.position;

    /*    Покрыть всё в одно движение
     *
     *    Начать двигать какой-то мышцой (варьировать длительность),
     *    К её движению сложить все комбинации остальных мышц.
     *
     */

    /* Возьмём первый мускул наугад */
    for ( auto muscle_i : hand.muscles_ /* | boost::adaptors::sliced (3, 4) */ )
    {
      // if ( !(muscle_i & muscles) && muscleValidAtOnce(muscle_i | muscles) )
      {
        HandMoves::trajectory_t  trajectory;
        /* Попробуем его варьировать по длительности */
        for ( Hand::frames_t last_i : boost::irange<Hand::frames_t> (1U, hand.maxMuscleLast (muscle_i) / 2, 10) )
        {
          Hand::Control control_i (muscle_i, 0U, last_i);
          // hand.move ({ control_i });

          for ( auto muscle_j : hand.muscles_ )
            if ( (muscle_i != muscle_j) && muscleValidAtOnce (muscle_i | muscle_j) )
            {
              for ( Hand::frames_t start_j : boost::irange<Hand::frames_t> (1U, last_i, 10) ) // ?? + inertial_lasts
              {
                for ( Hand::frames_t last_j : boost::irange<Hand::frames_t> (1U, hand.maxMuscleLast (muscle_j) / 2, 10) )
                {
                  Hand::Control control_j (muscle_j, 0U /* start_j */, last_j);
                  hand.move ({ control_i, control_j }, &trajectory);

                  // auto& index = store.get<Record::ByC> ();
                  // 
                  // auto& index = store.get<Record::ByP> ();
                  // index.equal_range (hand.position);

                  Point hand_pos = hand.position;
                  HandMoves::Record  rec (hand_pos, hand_base, hand_pos,
                                          { control_i, control_j }, trajectory);

                  store.insert (rec);
                  // trajectories.push_back (make_shared<trajectory_t> (trajectory));
                }
              }
            }
        }
      }
      // for ( auto muscle_j : hand.muscles_ )
      // {
      //   if ( !(muscle_j & muscles) && muscleValidAtOnce (muscle_i | muscles) )
      //   {
      //     for ( Hand::frames_t last_i : boost::irange<Hand::frames_t> (1U, 126U, 10) ) //hand.maxMuscleLast (muscle_i)) )
      //     {
      //       hand.move (muscle_i | muscle_j, last_i, trajectory);
      //       store.insert (Record (hand.position, hand_base, hand.position,
      //                    { muscle_i }, { 0 }, { last_i, 126U - last_i }, 1U,
      //                    trajectory)
      //                    );
      //     }
      //   } // end if
      // } // end for
      // 
      //   // trajectories.push_back (make_shared<trajectory_t> (trajectory));
      //   /*  Теперь в каждый такт длительности первого мускула,
      //    *  включим второй и будет варьировать его длительность
      //    */
      //   testCover (store, hand, muscles | muscle_i, trajectories);
      // 
      // } // end for
      //} // end if
    } // end for
  }
  //------------------------------------------------------------------------------
  class LearnMovements
  {
    /*  Количество точек, в окресности искомой точки.
     *
     *  Что я могу варьировать?
     *
     *  v  1. Длительность работы каждого мускула
     *  v  2. Время старта каждого мускула
     *  v  3. Тормозить мускулом? (прерывать '1' входа на которкое время)
     *
     *  ?  4. Может быть !1 перелом! траектории (полная остановка)
     *  Он может нам понадобиться в зависимости от законов движения
     *  разных мускулов, если один до второго не успевает ...
     *  (?позже включить??)
     *
     */

    /* stage_1 */
    //==============================================
    /* Желаемое расстояние между конеными точками
     * чернового покрытия
     */
    double             draft_distance = 0.07;
    /* Время преодоления инерции */
    Hand::frames_t   lasts_init_value1 = 50U;
    /* Изменение шага времени */
    Hand::frames_t   lasts_incr_value1 = 10U;
    /*  Примерное количество точек 
     *  от базового состояния до
     *  крайнего положения в каждую
     *  из возможных сторон движения.
     */
    // Hand::frames_t   count_points = 20U;
    //==============================================

    /* stage_2 */
    //==============================================
    double            detail_distance = 0.005;
    /* Время преодоления инерции */
    Hand::frames_t   lasts_init_value2 = 1U;
    /* Изменение шага времени */
    Hand::frames_t   lasts_incr_value2 = 1U;
    // Hand::frames_t   count_points = 20U;

    // class PredictedDirection
    // {
    //   Hand::MusclesEnum  muscle;
    //   Hand::frames_t     last;
    //   // trajectory_t trajectory; ???
    //   // Point center;
    // 
    //   PredictedDirection ():
    //     muscle (Hand::EmptyMov), last (0U), end (0,0) {}
    //   PredictedDirection (Hand::MusclesEnum m, Hand::frames_t l, Point e) :
    //     muscle (m), last (l), end (e) {}
    // 
    // public:
    //   Point end;
    // };
    class DirectionPredictor
    {
      //------------------------------------------
      class MainDirection
      {
      public:
        //------------------------------------------
        Hand::MusclesEnum  muscle;
        std::vector<Point>  shifting;
        Point  center;
        //------------------------------------------
        MainDirection (Hand::MusclesEnum m, Hand &hand) :
          muscle (m),
          center (hand.jointPosition (jointByMuscle (m)))
        {
          hand.SET_DEFAULT;
          Point hand_base = hand.position;

          std::list<Point>  trajectory;
          //-----Must be !!! EACH !!!--------
          auto oldEach = hand.visitedSaveEach;
          hand.visitedSaveEach = 1U;
          hand.move (m, hand.maxMuscleLast (m), trajectory /*, !!! */);
          hand.visitedSaveEach = oldEach;
          //---------------------------------
          for ( auto pt : trajectory )
          { shifting.push_back (Point (pt.x - hand_base.x,
                                       pt.y - hand_base.y));
          }
        }
        //------------------------------------------
        bool operator== (Hand::MusclesEnum m) const
        { return  (muscle == m); }
        bool operator!= (Hand::MusclesEnum m) const
        { return  (muscle != m); }
      };
      //------------------------------------------
      std::list<MainDirection>  mainDirections;
      Point hand_base;
      //------------------------------------------
    public:
      DirectionPredictor (Hand &hand) :
        hand_base (hand.position)
      {
        for ( auto m : hand.muscles_ )
        { mainDirections.push_back (MainDirection (m, hand)); }
      }
      //------------------------------------------
      /* Point */ void  predict (IN  Hand::MusclesEnum m,
                                  IN  Hand::frames_t l,
                                  OUT Point &end) const
      {
        // Point end = hand_base;
        // if ( /* m not in mainDirections */ 1 )
        {
          for ( auto m_ : muscles )
            for ( auto &md : mainDirections ) // !!!!!!!!!!!!
              if ( md == (m & m_) && l < md.shifting.size () )
              {
                end.x += md.shifting[l].x;
                end.y += md.shifting[l].y;
              }
        }
        // return end;
      }
      //------------------------------------------
    };

    std::map < Hand::MusclesEnum /* -------------muscle---*/,
               std::pair< Hand::frames_t /*---min lasts---*/ ,
                          Hand::frames_t /*---max lasts---*/ >
             > borders_;
    // !!! НУЖНО ТОРМОЗИТЬ УЖЕ НА ЭТОЙ СТАДИИ !!!
    //==============================================
    int count = 0;
    int count_TP = 0;
    int count_FP = 0;
    int count_TN = 0;
    int count_FN = 0;

    /* stage_3 */
    //==============================================
    //==============================================

  public:
    // LearnMovements () : lasts_incr_value1 (10U), lasts_incr_value2 (3U) {}
    // LearnMovements (HandMoves::Store &store, Hand &hand, RecTarget &target)
    // {
    //   lasts_incr_value1 = 0U;
    //   for ( auto m : hand.muscles_ )
    //     lasts_incr_value1 += hand.maxMuscleLast (m);
    //   lasts_incr_value1 /= (Hand::frames_t) hand.muscles_.size ();
    //   lasts_incr_value1 /= count_points;
    // }

    void  draftDistance (IN  Point  &hand_pos_prev,
                         IN  Point  &hand_pos,
                         OUT Hand::frames_t  &lasts_step,
                         IN  const Hand::frames_t  &last,
                         IN  const Hand::frames_t   lasts_max,
                         IN  const Hand::frames_t   lasts_incr_value,
                         IN double distance )
    {
      if ( boost_distance (hand_pos_prev, hand_pos) > distance )
      {
        if ( lasts_step > 2 * lasts_incr_value )
          lasts_step -= lasts_incr_value;
        // else if ( lasts_step > lasts_incr_value )
        //   lasts_step -= lasts_incr_value * 0.5;
        // else if ( lasts_step > 2 )
        //   --lasts_step;
      }
      else if ( boost_distance (hand_pos_prev, hand_pos) < distance )
      {
        if ( (last + lasts_incr_value) < lasts_max )
          lasts_step += lasts_incr_value;
        // else if ( (last + 0.5 * lasts_incr_value) < last_max )
        //   lasts_step += 0.5 * lasts_incr_value;
        // else
        //   lasts_step = lasts_max - last;
      }
    
    }
    void  borders_insert (const HandMoves::Record &rec)
    {
      for ( auto &ctrl : rec.controls () )
      {
        for ( auto m : muscles )
          if ( m & ctrl.muscle )
          {
            auto it = borders_.find (m);
            if ( it != borders_.end () )
            {
              if ( ctrl.last < it->second.first )
                it->second.first = ctrl.last;
              if ( ctrl.last > it->second.second )
                it->second.second = ctrl.last;
            }
            else
            { borders_[ctrl.muscle] = std::make_pair (ctrl.last, ctrl.last); }
          }
      } // end for
    }

    void  borders (HandMoves::Store &store, RecTarget &target)
    {
      for ( auto &rec : store )
      {
        if ( target.contain (rec.hit) )
        {
          borders_insert (rec);
        } // end if
      } // end for

      HandMoves::adjacency_refs_t range;
      store.adjacencyPoints (range, (target.min) (), draft_distance);
      for ( auto p_rec : range )
      {
        borders_insert (*p_rec);
      }

      range.clear ();
      store.adjacencyPoints (range, Point ((target.min) ().x, (target.max) ().y), draft_distance);
      for ( auto p_rec : range )
      {
        borders_insert (*p_rec);
      }

      range.clear ();
      store.adjacencyPoints (range, Point ((target.max) ().x, (target.min) ().y), draft_distance);
      for ( auto p_rec : range )
      {
        borders_insert (*p_rec);
      }

      range.clear ();
      store.adjacencyPoints (range, (target.max) (), draft_distance);
      for ( auto p_rec : range )
      {
        borders_insert (*p_rec);
      }

    }

    void  checking (Hand &hand, RecTarget &target, 
                    std::vector<Hand::MusclesEnum> &ms,
                    std::vector<Hand::frames_t> &lasts,
                    const Point &end)
    {
      std::vector<Hand::Control> controls (ms.size ());
      for ( size_t ii = 0U; ii < ms.size (); ++ii )
        controls[ii] = Hand::Control (ms[ii], 0U, lasts[ii]);

      HandMoves::trajectory_t  trajectory;
      hand.SET_DEFAULT;
      hand.move (controls.begin (), controls.end (), &trajectory);

      ++count;
      bool model = target.contain (end);
      bool real  = target.contain (hand.position);
      if ( model & real )
        ++count_TP;
      else if ( !model & !real )
        ++count_TN;
      else if ( model & !real )
        ++count_FP;
      else if ( !model & real )
        ++count_FN;
    }


    void runNextNestedMusclesFor (HandMoves::Store &store, Hand &hand, RecTarget &target,
                                  std::vector<Hand::MusclesEnum> &ms,
                                  std::vector<Hand::frames_t> &lasts,
                                  size_t nested)
    {
      for ( auto muscle : { muscleByJoint (hand.joints_[nested], false),
                            muscleByJoint (hand.joints_[nested], true) } )
      {
        ms[nested] = muscle;
        if ( nested < hand.joints_.size () )
          runNextNestedMusclesFor (store, hand, target, ms, lasts, nested + 1U);
        else
        { size_t  nested_lasts = 0U;
          runNextNestedLastsFor   (store, hand, target, ms, lasts, nested_lasts);
        } // end else
      } // end for
    }

    void runNextNestedLastsFor (HandMoves::Store &store, Hand &hand, RecTarget &target,
                                std::vector<Hand::MusclesEnum> &ms,
                                std::vector<Hand::frames_t> &lasts,
                                size_t nested)
    {
      for ( auto last : { muscleByJoint (hand.joints_[nested], false),
           muscleByJoint (hand.joints_[nested], true) } )
      {
        lasts[nested] = last;
        if ( nested < hand.joints_.size () )
          runNextNestedLastsFor (store, hand, target, ms, lasts, nested + 1U);
        else
          runNextInserting (store, hand, target, ms, lasts);
      } // end for
    }

    void  runNextInserting (HandMoves::Store &store, Hand &hand, RecTarget &target,
                            std::vector<Hand::MusclesEnum> &ms,
                            std::vector<Hand::frames_t> &lasts)
    {
      hand.SET_DEFAULT;
      Point hand_base = hand.position;
      DirectionPredictor  pd (hand);
      Point end = hand_base;

      for ( auto index : boost::irange (size_t{ 0U }, hand.joints_.size ()) )
        pd.predict (ms[index], lasts[index], end);

      checking (hand, target, ms, lasts, end);

      if ( target.contain (end) )
      {
        /* !!! НА МИШЕНИ !!! */
        std::list<Hand::Control> controls;
        for ( auto ii = 0U; ii < ms.size (); ++ii )
          controls.push_back ( Hand::Control (ms[ii], 0U, lasts[ii]) );

        HandMoves::trajectory_t  trajectory;
        hand.move (controls.begin (), controls.end (), &trajectory);
        Point hand_pos = hand.position;
        hand.SET_DEFAULT;

        /// ??????????????????
        // if ( last_j == last_init )
        //   hand_pos_i = hand_pos;

        HandMoves::Record  rec (hand_pos, hand_base, hand_pos,
                                controls, trajectory);
        store.insert (rec);

        /// ??????????????????
        // draftDistance (hand_pos_prev_j, hand_pos, lasts_step, last_j, last_j_max, lasts_incr_value2);
        // hand_pos_prev_j = hand_pos;

        boost::this_thread::interruption_point ();
      } /* end if */
    }

    //------------------------------------------------------------------------------
    /* грубое покрытие всего рабочего пространства */
    void  testStage1 (HandMoves::Store &store, Hand &hand, RecTarget &target)
    {
      size_t complexity = 0U;

      hand.reset ();
      hand.SET_DEFAULT;

      Point  hand_pos_base = hand.position;

      Point  hand_pos_prev_i = hand_pos_base;
      Point  hand_pos_prev_j = hand_pos_base;

      /* Возьмём первый мускул наугад */
      for ( auto muscle_i : { muscleByJoint (hand.joints_[0], false), muscleByJoint (hand.joints_[0], true) } ) // hand.muscles_
        for ( auto muscle_j : { muscleByJoint (hand.joints_[1], false), muscleByJoint (hand.joints_[1], true) } ) // hand.muscles_
      //for ( auto muscle_i : hand.muscles_ )
      //  for ( auto muscle_j : hand.muscles_ )
          if ( (muscle_i != muscle_j) && muscleValidAtOnce (muscle_i | muscle_j) )
          {
            Point  hand_pos_i;
            /* Нужен уменьшающийся шаг в зависимости от пройдённой дистанции */
            Hand::frames_t  lasts_step = lasts_init_value1;

            auto last_i_max = hand.maxMuscleLast (muscle_i),
                 last_j_max = hand.maxMuscleLast (muscle_j);
            /* Попробуем его варьировать по длительности */
            for ( Hand::frames_t last_i = lasts_init_value1; last_i < last_i_max; last_i += lasts_step )
            {
              for ( Hand::frames_t last_j = lasts_init_value1; last_j < last_j_max; last_j += lasts_step )
              {
                Hand::Control  control_i (muscle_i, 0U, last_i);
                // {
                  //   trajectory_t  trajectory;
                  //   hand.SET_DEFAULT;
                  //   hand.move ({ control_i }, &trajectory);
                  //   store.insert (Record (hand.position, hand_base, hand.position,
                  //                          { muscle_i }, { 0U }, { last_i }, 1U,
                  //                            trajectory));
                  // }
                Hand::Control  control_j (muscle_j, 0U /* start_j */, last_j);

                HandMoves::trajectory_t  trajectory;
                hand.SET_DEFAULT;
                hand.move ({ control_i, control_j }, &trajectory);

                Point hand_pos_j = hand.position;
                if ( last_j == lasts_init_value1 )
                  hand_pos_i = hand_pos_j;

                HandMoves::Record  rec (hand_pos_j, hand_pos_base, hand_pos_j,
                                        { control_i, control_j }, trajectory);
                store.insert (rec);

                draftDistance (hand_pos_prev_j, hand_pos_j, lasts_step, last_j, last_j_max, lasts_incr_value1, draft_distance);
                hand_pos_prev_j = hand_pos_j;
                
                ++complexity;

                try
                { boost::this_thread::interruption_point (); }
                catch ( boost::thread_interrupted& )
                { return; } // end catch

              } // end for

              draftDistance (hand_pos_prev_i, hand_pos_i, lasts_step, last_i, last_i_max, lasts_incr_value1, draft_distance);
              hand_pos_prev_i = hand_pos_i;

            } // end for 
          }
      // ----------------------------------------------------
      std::wcout << _T("\nStage 1 Complexity ") << complexity << std::endl;
      // ----------------------------------------------------
    }
    /* Покрытие всей мишени не слишком плотно */
    void  testStage2 (HandMoves::Store &store, Hand &hand, RecTarget &target)
    {
      /*         ~ - - - - *
       *       /
       *      /  +----------------+
       *    x \  |                |
       *   x   \ |                |
       * x       -                |
       *         | \              |
       *         |  *             |
       *         |                |
       *         +----------------+
       */
      size_t complexity = 0U;

      borders (store, target);
      DirectionPredictor  pd (hand);

      hand.reset ();
      hand.SET_DEFAULT;

      Point  hand_pos_base = hand.position;

      Point  hand_pos_prev_i = hand_pos_base;
      Point  hand_pos_prev_j = hand_pos_base;

      // for ( size_t i : boost::irange<size_t> (0U, hand.controlsCount) )
      // { Hand::MusclesEnum  m = hand.selectControl (i); }

      count = 0;
      count_TP = 0;
      count_FP = 0;
      count_TN = 0;
      count_FN = 0;

      draft_distance = 0.1;
      Hand::frames_t  last_step = 10U;

      // std::wcout << _T ("TARGET: ") << tstring ((target.min) ()) << _T ("  ") << tstring ((target.max) ()) << std::endl;

      // try
      // { 
      //   std::vector<Hand::MusclesEnum>  ms (hand.joints_.size ());
      //   std::vector<Hand::frames_t>  lasts (hand.joints_.size ());
      //   
      //   runNextNestedMusclesFor (store, hand, target, ms, lasts, 0U);
      // }
      // catch ( boost::thread_interrupted& )
      // { return; } // end catch

      for ( auto muscle_i : { muscleByJoint (hand.joints_[0], false), muscleByJoint (hand.joints_[0], true) }  ) // hand.muscles_
        for ( auto muscle_j : { muscleByJoint (hand.joints_[1], false), muscleByJoint (hand.joints_[1], true) } ) // hand.muscles_
      // for ( auto muscle_i : hand.muscles_ )
      //   for ( auto muscle_j : hand.muscles_ )
          if ( (muscle_i != muscle_j) && muscleValidAtOnce (muscle_i | muscle_j) )
          {
            Point  hand_pos_i;
            /* Нужен уменьшающийся шаг в зависимости от пройдённой дистанции */
            Hand::frames_t  last_step = lasts_incr_value2;

            auto  last_i_max = hand.maxMuscleLast (muscle_i),
                  last_j_max = hand.maxMuscleLast (muscle_j);

            bool target_contain = true;

            for ( Hand::frames_t last_i = /*lasts_init_value2*/borders_[muscle_i].first /*- 50*/; 
                                        target_contain || last_i <= borders_[muscle_i].second /*+ 50*/; last_i += last_step )
            // for ( Hand::frames_t last_i = 1U; last_i <= last_i_max; last_i += last_step )
            {
              for ( Hand::frames_t last_j = /*lasts_init_value2*/borders_[muscle_j].first /*- 50*/; 
                                             target_contain || last_j <= borders_[muscle_j].second /*+ 50*/; last_j += last_step )
              // for ( Hand::frames_t last_j = 1U; last_j <= last_j_max; last_j += last_step)
              {
                Point end = hand_pos_base;
                pd.predict (muscle_i, last_i, end);
                pd.predict (muscle_j, last_j, end);

                checking (hand, target,
                          std::vector<Hand::MusclesEnum>{ muscle_i, muscle_j },
                          std::vector<Hand::frames_t>{ last_i, last_j }, end);
                
                if ( target.contain (end) )
                {
                  /* !!! НА МИШЕНИ !!! */
                  Hand::Control  control_i (muscle_i, 0U, last_i);
                  Hand::Control  control_j (muscle_j, 0U /* start_j */, last_j);

                  HandMoves::trajectory_t  trajectory;
                  hand.SET_DEFAULT;
                  hand.move ({ control_i, control_j }, &trajectory);

                  Point  hand_pos_j = hand.position;
                  if ( last_j == lasts_init_value2 )
                    hand_pos_i = hand_pos_j;
                  //----------------------------------------------
                  target_contain = target.contain (hand_pos_j);
                  // std::wcout << tstring(hand_pos_j) << std::endl;
                  //----------------------------------------------
                  HandMoves::Record  rec (hand_pos_j, hand_pos_base, hand_pos_j,
                  { control_i, control_j }, trajectory);
                  store.insert (rec);

                  draftDistance (hand_pos_prev_j, hand_pos_j, last_step, last_j, last_j_max, lasts_incr_value2, detail_distance);
                  hand_pos_prev_j = hand_pos_j;
                  
                  ++complexity;

                  try
                  { boost::this_thread::interruption_point (); }
                  catch ( boost::thread_interrupted& )
                  { return; } // end catch

                }
                else
                { target_contain = false; }
              } // end for

              draftDistance (hand_pos_prev_i, hand_pos_i, last_step, last_i, last_i_max, lasts_incr_value2, detail_distance);
              hand_pos_prev_i = hand_pos_i;
            } // end for
          } // end if
    // } // end for

      if ( 1 )
      {
        std::cout << "count = " << count << std::endl;
        std::cout << "TP = " << count_TP << std::endl;
        std::cout << "TN = " << count_TN << std::endl;
        std::cout << "FP = " << count_FP << std::endl;
        std::cout << "FN = " << count_FN << std::endl;

        std::cout << "\t< T >\t < F >" << std::endl;
        std::cout << "< P > " << count_TP << '\t' << count_FP << std::endl;
        std::cout << "< N > " << count_TN << '\t' << count_FN << std::endl;
      }
      // ----------------------------------------------------
      std::wcout << _T ("\nStage 2 Complexity ") << complexity << std::endl;
      // ----------------------------------------------------
      hand.SET_DEFAULT;
    }
    /* Попадание в оставшиеся непокрытыми точки мишени */
    void  testStage3 (HandMoves::Store &store, Hand &hand, RecTarget &target, std::list<Point> &uncovered)
    {
      // std::list<Point> uncovered;
      // store.uncoveredTargetPoints (target, uncovered);
      for ( auto &pt : target.coords () )
      {
        std::list<std::shared_ptr<HandMoves::Record>> range;
        store.adjacencyPoints (range, pt, 2. * target.thickness ());

        if ( range.empty () )
        {
          /* UNCOVERED */
          uncovered.push_back (pt);
        }
        else
        {
          double precision = target.precision ();
          if ( boost::algorithm::none_of (range, [&pt, precision](const std::shared_ptr<HandMoves::Record> &rec)
                                                 { return  (boost_distance (pt, rec->hit) <= precision); }) )            
          {
            /* UNCOVERED */
            uncovered.push_back (pt);

          } // end if

        } // end else
      } // end for
    }

    void  testCoverTarget (HandMoves::Store &store, Hand &hand, RecTarget &target)
    {
      hand.reset ();
      hand.SET_DEFAULT;
   
      Point hand_base = hand.position;
      Point hand_prev = hand.position;
   
      /*    Покрыть всё в одно движение
       *
       *    Начать двигать какой-то мышцой (варьировать длительность),
       *    К её движению сложить все комбинации остальных мышц.
       */
      try
      {
#ifdef    _HAND_TEST_CONSOLE_PRINTF
        std::wcout << _T( "hand.muscles_.size = " ) << (int) hand.muscles_.size () << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
        /* Возьмём первый мускул наугад */
        for ( auto muscle_i : hand.muscles_ )
        {
#ifdef    _HAND_TEST_CONSOLE_PRINTF
          std::wcout << muscle_i << _T ("  ") << (int) hand.maxMuscleLast (muscle_i) << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
            for ( auto muscle_j : hand.muscles_ )
            {
              if ( (muscle_i != muscle_j) && muscleValidAtOnce (muscle_i | muscle_j) )
              {
#ifdef    _HAND_TEST_CONSOLE_PRINTF
                std::wcout << muscle_j << _T ("  ") << (int) hand.maxMuscleLast (muscle_j) << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
                /* Попробуем его варьировать по длительности */
       
                // for ( Hand::frames_t last_i : boost::irange<Hand::frames_t> (5U, hand.maxMuscleLast (muscle_i), 2) )
                for ( Hand::frames_t  last_i : boost::irange<Hand::frames_t> (5U, hand.maxMuscleLast (muscle_i)) )
                {
                  Hand::Control control_i (muscle_i, 0U, last_i);
                  // hand.move ({ control_i });
       
                  // for ( Hand::frames_t last_j : boost::irange<Hand::frames_t> (3U, hand.maxMuscleLast (muscle_j), 2) )
                  for ( Hand::frames_t last_j : boost::irange<Hand::frames_t> (3U, hand.maxMuscleLast (muscle_j)) )
                  {
                    // for ( Hand::frames_t start_j : boost::irange<Hand::frames_t> (5U, last_i, 2) ) // ?? + inertial_lasts
                    for ( Hand::frames_t  start_j : boost::irange<Hand::frames_t> (5U, last_i) )
                    {
                      boost::this_thread::interruption_point ();
       
                      HandMoves::trajectory_t  trajectory;
                      Hand::Control control_j (muscle_j, start_j, last_j);
       
                      hand.SET_DEFAULT;
                      hand.move ({ control_i, control_j }, &trajectory);
       
                      // if ( hand.position.x > x_leftBorder && hand.position.x < x_rightBorder
                      //   && hand.position.y < y_topBorder && hand.position.y > y_bottomBorder )
       
                      const Point&  hand_pos = hand.position;
                      if ( target.contain (hand_pos) )
                      {
                        store.insert (HandMoves::Record (hand_pos, hand_base, hand_pos,
                                                         { muscle_i, muscle_j }, { 0, start_j }, { last_i, last_j }, 2U,
                                                         trajectory));
                        // trajectories.push_back (make_shared<trajectory_t> (trajectory));
       
                             if ( store.size () == 200000 )
                        {
                          store.save (_T ("NewHand_200000_moves_tightly_save.bin"));
#ifdef    _HAND_TEST_CONSOLE_PRINTF
                          std::wcout << _T ("NewHand_200000_moves_tightly_saved") << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
                        }
                        else if ( store.size () == 400000 )
                        {
                          store.save (_T ("NewHand_400000_moves_tightly_save.bin"));
#ifdef    _HAND_TEST_CONSOLE_PRINTF
                          std::wcout << _T ("NewHand_400000_moves_tightly_saved") << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
                        }
                        else if ( store.size () == 600000 )
                        {
                          store.save (_T ("NewHand_600000_moves_tightly_save.bin"));
#ifdef    _HAND_TEST_CONSOLE_PRINTF
                          std::wcout << _T ("NewHand_600000_moves_tightly_saved") << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
                        }
                        else if ( store.size () == 1000000 )
                        {
                          store.save (_T ("NewHand_1000000_moves_tightly_save.bin"));
#ifdef    _HAND_TEST_CONSOLE_PRINTF
                        std::wcout << _T ("NewHand_1000000_moves_tightly_saved") << std::endl;
                        std::wcout << muscle_i << ' ' << last_i << std::endl;
                        std::wcout << muscle_j << ' ' << start_j << ' ' << last_j << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
                        }
                        else if ( store.size () == 1500000 )
                        {
                          store.save (_T ("NewHand_1500000_moves_tightly_save.bin"));
#ifdef    _HAND_TEST_CONSOLE_PRINTF
                        std::wcout << _T ("NewHand_1500000_moves_tightly_saved") << std::endl;
                        std::wcout << muscle_i << ' ' << last_i << std::endl;
                        std::wcout << muscle_j << ' ' << start_j << ' ' << last_j << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
                        }
                        else if ( store.size () == 2000000 )
                        {
                          store.save (_T ("NewHand_2000000_moves_tightly_save.bin"));
#ifdef    _HAND_TEST_CONSOLE_PRINTF
                        std::wcout << _T ("NewHand_2000000_moves_tightly_saved")  << std::endl;
                        std::wcout << muscle_i << ' '                   << last_i << std::endl;
                        std::wcout << muscle_j << ' ' << start_j << ' ' << last_j << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
                        }
                      }
                   
#define   _HAND_TEST_BREAK
#ifdef    _HAND_TEST_BREAK
                      if ( boost_distance (hand_pos,  target.center ())
                         > boost_distance (hand_prev, target.center ()) )
                      {
                        // #ifdef    _HAND_TEST_CONSOLE_PRINTF
                        // std::wcout << _T ("break") << std::endl;
                        // #endif // _HAND_TEST_CONSOLE_PRINTF
                        break;
                      }
                      hand_prev = hand_pos;
#endif // _HAND_TEST_BREAK
                      try
                      { boost::this_thread::interruption_point (); }
                      catch ( boost::thread_interrupted& )
                      {
#ifdef    _HAND_TEST_CONSOLE_PRINTF
                        std::wcout << _T ("WorkingThread interrupted")            << std::endl;
                        std::wcout << muscle_i << ' '                   << last_i << std::endl;
                        std::wcout << muscle_j << ' ' << start_j << ' ' << last_j << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
                        return;
                      } // end catch
                    } // end for
                  } // end for
                } // end for
              } // end if
            } // end for
        } // end for
      } // end try
      catch ( std::exception &ex )
      { 
#ifdef    _HAND_TEST_CONSOLE_PRINTF
        std::cout << ex.what () << std::endl;
#endif // _HAND_TEST_CONSOLE_PRINTF
        return;
      }
    }
    //------------------------------------------------------------------------------
    struct Direction
    {
      double             angle; // with Ox
      Point              direction;
      Hand::MusclesEnum  control;
      Point              norm_dir;
      
      struct Ellipse
      {


      };

      Direction () :
        direction (0., 0.),
        control (Hand::EmptyMov),
        angle (0.)
      {}

      Direction (Point direction, Hand::MusclesEnum control) :
        direction (direction),
        control (control),
        angle (atan2 (direction.y, direction.x))
      {}

      double  cos_sim (const Direction &d) const
      {
        auto  ad = direction;
        auto  bd = d.direction;
        /* cosine similarity */
        return    (ad.x * bd.x + ad.y * bd.y)
          / (sqrt (ad.x * ad.x + ad.y * ad.y)
             *  sqrt (bd.x * bd.x + bd.y * bd.y));
      }

      void draw (HDC hdc)
      {
        if ( !norm_dir.x && !norm_dir.y )
          norm_dir = Point (0.1 * cos (angle), 0.1 * sin (angle));

        MoveToEx (hdc, Tx (0.), Ty (0.), NULL);
        LineTo (hdc, Tx (norm_dir.x), Ty (norm_dir.y));
      }

    };

    void  getTargetCenter (Hand &hand, Point &center)
    {
      hand.SET_DEFAULT;
      Point start = hand.position;
    
     
    
      std::vector<Direction>  directions;
      directions.reserve (hand.muscles_.size ());
    
      //hand.selectControl ()
      // for ( auto muscle_i : hand.muscles_ )
      for ( auto iCtrl = 0U; iCtrl < hand.controlsCount; ++iCtrl )
      {
        auto muscle_i = hand.selectControl (iCtrl);
    
        hand.move (muscle_i, 1U);
        // hand.step (muscle_i, 1U); hand.step ();
    
        directions.push_back (Direction (Point (hand.position.x - start.x,
                              hand.position.y - start.y), muscle_i));
      }
      // --------------------
      Direction   direct = Direction (Point (center.x - start.x,
                                      center.y - start.y), Hand::EmptyMov);
    
      Direction  &d = *std::min_element (directions.begin (), directions.end (),
                                         [&direct](const Direction &a, const Direction &b)
                                         { return  direct.cos_sim (a) < direct.cos_sim (b); });
                                         // { return  boost_distance (direct.direction, a.direction)
                                         //         > boost_distance (direct.direction, b.direction); });
      // --------------------
    
      auto max_last = hand.maxMuscleLast (d.control);
      hand.move (d.control, max_last);
    
      double center_distance = boost_distance (center, start);
      double maxdir_distance = boost_distance (hand.position, start);
    
      Hand::frames_t  last = static_cast<Hand::frames_t> ((center_distance * max_last) / maxdir_distance);
      hand.move (d.control, last);
    }
    //------------------------------------------------------------------------------
    bool  tryToHitTheAim (HandMoves::Store &store, Hand &hand, const Point &aim,
                          size_t tries, double precision)
    {
      bool result = 0;
      std::list<std::shared_ptr<HandMoves::Record>>  exact_range;
      /* While there is no exact trajectory */
      for ( size_t n_try = 0U; (result = exact_range.empty ()) || n_try < tries; ++n_try )
      {
        store.adjacencyPoints (exact_range, aim, precision);

      }
      return !result;
    }
  };

   static void  testCoverTarget1 (HandMoves::Store &store, Hand &hand, RecTarget &target)
  {
    hand.SET_DEFAULT;
    // std::list<std::list<Point>> trajectories;

    /* Fixate the aim point */
    boost_point2_t  aim = target.coords ()[45];
    double prev_distance = boost::geometry::distance (aim, hand.pos);

    double             best_distance = prev_distance;
    Hand::MusclesEnum  best_muscle = Hand::EmptyMov;
    Hand::time_t       best_last;

    // while ( aim.y () < hand.position.y )
    {
      /* Try each muscle */
      for ( auto i : boost::irange<size_t> (1U, hand.controlsCount) )
      {
        auto muscle = hand.selectControl (i);
        auto last = hand.maxMuscleLast (muscle);

        hand.step (muscle);
        hand.step ();
        /* Find the best muscle by distance */
        double cur_distance = boost::geometry::distance (hand.pos, aim);
        if ( cur_distance < best_distance )
        {
          best_muscle = muscle;
          best_last = last;
          best_distance = cur_distance;
        }

        hand.reset ();
        hand.SET_DEFAULT;
      }

      hand.step (best_muscle);
      // while ( last-- )
      prev_distance = best_distance;
      double cur_distance = best_distance;
      auto last_ = 1U;

      do
        // for ( auto last_ = 1U; (prev_distance >= cur_distance) && (last_ <= best_last); ++last_ )
      {
        prev_distance = cur_distance;
        //-----------
        hand.step ();
        //-----------
        cur_distance = boost::geometry::distance (hand.pos, aim);
        ++last_;
      } while ( (prev_distance >= cur_distance) && (last_ <= best_last) );

      if ( !hand.moveEnd )
        hand.step (best_muscle);

      while ( !hand.moveEnd )
        hand.step ();
    }
    //=========================================================================
    // // const to target
    // const boost_point2_t  tlu (target.Min ().x, target.Max ().y),
    //                       tld (target.Min ().x, target.Min ().y),
    //                       trd (target.Max ().x, target.Min ().y),
    //                       tru (target.Max ().x, target.Max ().y);
    // 
    // const array<boost_point2_t, 4> target_corners = { tlu, tld, trd, tru };
    // const boost_point2_t *t_closest;
    // double distance = 0.;

    /* Create the tree of possible passes */
    //for ( Hand::MusclesEnum muscle_i : Hand::muscles )
    //{
    //  // auto h = hand.position;

    //  for ( uint_t last_i = 1U; last_i < hand.maxMuscleLast (muscle_i); ++last_i )
    //  {
    //    std::list<Point> trajectory;
    //    
    //    
    //    const boost_point2_t pos = hand.position;
    //    t_closest = &target_corners[0];
    //    distance = boost::geometry::distance (t_closest, pos);
    //    for ( size_t i = 1U; i < target_corners.size (); ++i )
    //    {
    //      double d = boost::geometry::distance (target_corners[i], pos);
    //      if ( distance > d )
    //      {
    //        distance = d;
    //        t_closest = &target_corners[i];
    //      }
    //    }

    //    // hand.move (muscle_i, last_i, trajectory);
    //    hand.step (muscle_i);
    //    // while ( last-- )
    //    for ( auto last_ = 1U; last_ <= last_i; ++last_ )
    //    {
    //      hand.step ();

    //      const boost_point2_t pos = hand.position;
    //      double d = boost::geometry::distance (pos, t_closest);
    //    }
    //    if ( !hand.isMoveEnd () )
    //      hand.step (muscle_i);

    //    while ( !hand.isMoveEnd () )
    //      hand.step ();

    //{
    //  const Point &aim = hand.position;
    //  Record rec (aim, aim,
    //              { muscle_i }, { 0 }, { last_i },
    //              1U, trajectory);
    //  store.insert (rec);
    //}
    //=========================================================================
    //// std::cout << muscle_i << ' ' << hi << ' ' << last_i << std::endl;
    //insert (trajectories, trajectory);

    //if ( nesting > 1U )
    //  for ( Hand::MusclesEnum muscle_j : Hand::muscles )
    //  {
    //    if ( (muscle_i == muscle_j) || !muscleValidAtOnce (muscle_i | muscle_j) )
    //      // if ( j == i )
    //      continue;

    //    Point cur1 = hand.position;

    //    for ( uint_t last_j = 1U; last_j < hand.maxMuscleLast (muscle_j) / 2; ++last_j )
    //    {
    //      std::list<Point>::iterator tail_j = trajectory.end (); --tail_j;

    //      hand.move (muscle_j, last_j, trajectory);
    //      {
    //        const Point &aim = hand.position;
    //        store.insert (Record (aim, aim,
    //        { muscle_i, muscle_j },
    //        { 0, last_i },
    //        { last_i, last_j },
    //                      2U,
    //                      trajectory)
    //                      );
    //      }

    //      // std::cout << muscles[j] << ' ' << hj << ' ' << last_j << std::endl;
    //      ++tail_j;

    //      // std::list::splice () allows you to concatenate two std::list's in constant time.
    //      insert (trajectories, trajectory);
    //      //=================================================
    //      if ( nesting > 2U )
    //        for ( Hand::MusclesEnum muscle_k : Hand::muscles )
    //        {
    //          if ( (muscle_i == muscle_k || muscle_k == muscle_j)
    //              || !muscleValidAtOnce (muscle_i | muscle_j | muscle_k) )
    //            // if ( k == i || k == j )
    //            continue;

    //          for ( auto last_k = 1U; last_k < hand.maxMuscleLast (muscle_k) / 2; ++last_k )
    //          {
    //            std::list<Point>::iterator tail_k = trajectory.end (); --tail_k;
    //            hand.move (muscle_k, last_k, trajectory);
    //            {
    //              const Point &aim = hand.position;
    //              store.insert (Record (aim, aim,
    //              { muscle_i, muscle_j, muscle_k },
    //              { 0, last_i, last_i + last_j },
    //              { last_i, last_j, last_k },
    //                            3U,
    //                            trajectory)
    //                            );
    //            }

    //            // std::cout << muscles[k] << ' ' << hk << ' ' << last_k << std::endl;
    //            ++tail_k;
    //            insert (trajectories, trajectory);
    //            trajectory.erase (tail_k, trajectory.end ());

    //            hand.set (jointByMuscle (muscle_k), { (jointByMuscle (muscle_k) == Hand::Elbow) ? 70. : 0. });
    //          }
    //        }
    //      //=================================================
    //      trajectory.erase (tail_j, trajectory.end ());
    //      //a.insert (a.end (), b.begin (), b.end ());

    //      hand.set (jointByMuscle (muscle_j), { (jointByMuscle (muscle_j) == Hand::Elbow) ? 70. : 0. });
    //    }
    //  }
    //     hand.set (jointByMuscle (muscle_i), { (jointByMuscle (muscle_i) == Hand::Elbow) ? 70. : 0. });
    //   }
    // }
    //hand.SET_DEFAULT;


  }
  //------------------------------------------------------------------------------
};
#endif // _POSITION_H_
