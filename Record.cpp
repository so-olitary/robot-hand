﻿#include "StdAfx.h"
#include "HandMovesStore.h"

using namespace std;
using namespace HandMoves;
//---------------------------------------------------------
Record::Record (const Point         &aim,
                const Point         &hand_begin,
                const Point         &hand_final,
                const muscles_array &muscles,
                const times_array   &times,
                const times_array   &lasts,
                size_t               controls_count,
                const trajectory_t  &visited) :
  aim_ (aim), hand_begin_ (hand_begin), hand_final_ (hand_final),
  muscles_ (Hand::EmptyMov), visited_ (visited)
{
  if ( !visited.size () )
    throw new exception ("Incorrect trajectory in constructor Record"); // _T( ?? 

  if ( controls_count > maxControlsCount )
    throw new exception ("Incorrect number of muscles in constructor Record"); // _T( ?? 

  for ( auto i = 0U; i < controls_count; ++i )
  { hand_controls_.push_back (Hand::Control (muscles[i], times[i], lasts[i])); }
  hand_controls_.sort ();

  auto  first_start_time = times.front ();
  for ( auto &hc : hand_controls_ )
  {
    muscles_ = muscles_ | hc.muscle;
    hc.start -= first_start_time;
  }

  if ( !validateMusclesTimes () )
    throw new std::exception ("Invalid muscles constructor Record parameter"); // _T( ??
}

Record::Record (const Point         &aim,
                const Point         &hand_begin,
                const Point         &hand_final,
                const std::list<Hand::Control> controls,
                const trajectory_t  &visited) :
  aim_ (aim), hand_begin_ (hand_begin), hand_final_ (hand_final),
  muscles_ (Hand::EmptyMov), visited_ (visited)
{
  if ( !visited.size () )
    throw new exception ("Incorrect trajectory in constructor Record"); // _T( ?? 

  if ( controls.size () > maxControlsCount )
    throw new exception ("Incorrect number of muscles in constructor Record"); // _T( ?? 

  hand_controls_.assign (controls.begin (), controls.end ());
  hand_controls_.sort ();

  auto  first_start_time = hand_controls_.begin ()->start;
  for ( auto &hc : hand_controls_ )
  {
    muscles_ = muscles_ | hc.muscle;
    hc.start -= first_start_time;
  }

  if ( !validateMusclesTimes () )
    throw new std::exception ("Invalid muscles constructor Record parameter"); // _T( ?? 
}
//---------------------------------------------------------
bool    Record::validateMusclesTimes () const
{
  if ( hand_controls_.size () > 1U )
  {
    for ( auto iti = hand_controls_.begin (); iti != hand_controls_.end (); ++iti )
      for ( auto itj = std::next (iti); itj != hand_controls_.end (); ++itj )
      {
        if ( itj->start >= (iti->start + iti->last) )
          break; // т.к. отсортированы по возрастанию start

        /* Если есть перекрытие по времени */
        if ( ( (iti->start <= itj->start) && (itj->start <= (iti->start + iti->last)) )
          || ( (itj->start <= iti->start) && (iti->start <= (itj->start + itj->last)) ) )
        {
          for ( auto j : joints )
          { Hand::MusclesEnum  Opn = muscleByJoint (j, true);
            Hand::MusclesEnum  Сls = muscleByJoint (j, false);
            /* Одновременно работающие противоположные мышцы */
            if ( (Opn & iti->muscle) && (Сls & itj->muscle) )
            { return false; } // end if
          } // end for
        } // end if
      } // end for-for
  } // end if
  return true;
}
//---------------------------------------------------------
double  Record::eleganceMove (/* const Point &aim */) const
{
  /*  max.отклонение      !!!ПЕРЕЛОМЫ траектории!!!
   *  Длина траектории по сравнениею с дистанцией
   */
  if ( 0 )
  {
    /* Суммарное время работы двигателей */
    auto sum_time_muscles = 0.;
    for ( auto &hc : hand_controls_ ) { sum_time_muscles += hc.last; }
    /* Количество движений */
    double controls_count_ratio = 1. / controlsCount;

    /* max.отклонение от оптимальной траектории */
    double max_divirgence = ratioTrajectoryDivirgence ();
    /* Количество задействованных мышц */
    double muscles_count_ratio = ratioUsedMusclesCount ();
  }
  /* Длина траектории по сравнениею с дистанцией */
  // double  distance_ratio = ratioDistanceByTrajectory ();
  
  // return  distance_ratio;
  /*   + max_divirgence + sum_time_muscles
   *   + muscles_count_ratio + controls_count_ratio
   */

  auto longest_control = *boost::max_element (controls (), [](const Hand::Control &a, const Hand::Control &b)
                                                           {return (a.start + a.last) > (b.start + b.last); });
  double total_time = (longest_control.start + longest_control.last) / 300.;
  // double total_time = (controls ().back ().start + controls ().back ().last) - controls ().front ().start;
  return total_time;
}
//---------------------------------------------------------
double  Record::ratioDistanceByTrajectory () const
{
  double  distance_ratio = 1.;
  double visited_disance = 0.;

  if ( visited_.size () )
  {
    auto curr = hand_begin_;
    /* Длина траектории по сравнениею с дистанцией */
    for ( auto next = visited_.begin () /*, next = std::next (curr)*/; next != visited_.end (); ++next )
    { visited_disance += boost_distance (curr, *next);
      curr = *next;
    }
    visited_disance += boost_distance (aim_, visited_.back ());

    distance_ratio = (visited_disance) ? (boost_distance (aim_, hand_begin_) / visited_disance) : 1.;
  }
  // else distance_ratio = 1.;
  return  distance_ratio;
}
double  Record::ratioTrajectoryDivirgence () const
{
  /* max.отклонение */
  typedef boost::geometry::model::d2::point_xy<double> bpt;
  boost::geometry::model::linestring<bpt>  line;

  auto start = visited_.front ();
  line.push_back (bpt (start.x, start.y));
  line.push_back (bpt (aim.x, aim.y));

  double max_divirgence = 0.;
  for ( auto &pt : visited_ )
  {
    double dist = boost::geometry::distance (bpt (pt.x, pt.y), line);
    if ( dist > max_divirgence )
      max_divirgence = dist;
  }
  
  return max_divirgence;
}

double  Record::ratioUsedMusclesCount () const
{
  /* Количество задействованных мышц */
  size_t muscles_count = 0U;
  for ( auto m : ::muscles )
    if ( m & muscles_ )
      ++muscles_count;
  return  (1. / muscles_count);
}
double  Record::ratioTrajectoryBrakes () const
{ /* ПЕРЛОМЫ */
  // остановки 

  // (1. / controlsCount) * /* Количество движений != ПЕРЕЛОМ */
  return 0.;
}
//---------------------------------------------------------
