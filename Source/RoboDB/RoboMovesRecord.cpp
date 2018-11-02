﻿#include "RoboMovesStore.h"

using namespace Robo;
using namespace RoboMoves;
//---------------------------------------------------------
RoboMoves::Record::Record(IN const Point         &aim,
                          IN const Point         &move_begin,
                          IN const Point         &move_final,
                          IN const muscles_array &muscles,
                          IN const frames_array  &starts,
                          IN const frames_array  &lasts,
                          IN size_t               controls_count,
                          IN const Trajectory    &visited) :
    aim_(aim), move_begin_(move_begin), move_final_(move_final), visited_(visited),
    _strategy(getStrategy(controls)),
    _lasts_step(0)
{
    if (!visited_.size())
        throw std::logic_error("Incorrect trajectory in constructor Record");

    for (auto i = 0U; i < controls_count; ++i)
        control_.append({ muscles[i], starts[i], lasts[i] });
    control_.validated(RoboI::musclesMaxCount);

    updateErrDistance(move_final);
}

RoboMoves::Record::Record(IN const Point            &aim,
                          IN const Point            &move_begin,
                          IN const Point            &move_final,
                          IN const Robo::Control    &controls,
                          IN const Robo::Trajectory &visited) :
    aim_(aim), move_begin_(move_begin), move_final_(move_final),
    control_(controls), visited_(visited), _lasts_step(0),
    _strategy(getStrategy(controls)),
    _update_time(std::time(NULL)), _update_traj(0)
{
    if (!visited_.size())
        throw std::logic_error("Record: Incorrect trajectory");

    //if (!control_.validateMusclesTimes())
    //    throw std::logic_error("Record: Invalid control");
    updateErrDistance(move_final_);
}

RoboMoves::Record::Record(IN const Point            &aim,
                          IN const Point            &move_begin,
                          IN const Point            &move_final,
                          IN const Robo::Control    &controls,
                          IN const Robo::Trajectory &visited,
                          IN Robo::frames_t lasts_step) :
    Record(aim, move_begin, move_final, controls, visited)
{
    _lasts_step = lasts_step;
    if (!_lasts_step)
        throw std::logic_error("Record: Incorrect lasts_step");
}
//---------------------------------------------------------
double  RoboMoves::Record::eleganceMove() const
{
    /* Суммарное время работы двигателей */
    double sum_time_muscles = 0.;
    for (size_t i = 0U; i < control_.size(); ++i)
    { sum_time_muscles += control_[i].lasts; }

    /* Количество движений */
    double controls_count_ratio = 1. / n_controls;
    /* Количество задействованных мышц */
    double muscles_count_ratio = ratioUsedMusclesCount();
    /* max.отклонение от оптимальной траектории */
    double max_divirgence = ratioTrajectoryDivirgence();
    /* Длина траектории по сравнениею с дистанцией */
    double distance_ratio = ratioDistanceByTrajectory();
    /* Переломы в движении */
    double trajectory_brakes = ratioTrajectoryBrakes();

    return  distance_ratio + max_divirgence
        + trajectory_brakes + sum_time_muscles
        + muscles_count_ratio + controls_count_ratio;

    // double total_time = longestMusclesControl ();
    // return total_time;
}
//---------------------------------------------------------
double  RoboMoves::Record::ratioDistanceByTrajectory() const
{
    double  distance_ratio = 1.;
    double visited_disance = 0.;

    if (visited_.size())
    {
        auto curr = move_begin_;
        /* Длина траектории по сравнениею с дистанцией */
        for (auto next = visited_.begin() /*, next = std::next (curr)*/; next != visited_.end(); ++next)
        {
            visited_disance += boost_distance(curr, *next);
            curr = *next;
        }
        visited_disance += boost_distance(aim_, visited_.back());

        distance_ratio = (visited_disance) ? (boost_distance(aim_, move_begin_) / visited_disance) : 1.;
    }
    // else distance_ratio = 1.;
    return  distance_ratio;
}
double  RoboMoves::Record::ratioTrajectoryDivirgence() const
{
    /* max.отклонение */
    typedef boost::geometry::model::d2::point_xy<double> bpt;
    boost::geometry::model::linestring<bpt>  line;

    auto start = visited_.front();
    line.push_back(bpt(start.x, start.y));
    line.push_back(bpt(aim.x, aim.y));

    double max_divirgence = 0.;
    for (auto &pt : visited_)
    {
        double dist = boost::geometry::distance(bpt(pt.x, pt.y), line);
        if (dist > max_divirgence)
            max_divirgence = dist;
    }

    return max_divirgence;
}
//---------------------------------------------------------
Robo::frames_t RoboMoves::Record::longestMusclesControl() const
{
    auto cmpAc = [](const Actuator &a, const Actuator &b) {
            return ((a.start + a.lasts) < (b.start + b.lasts));
        };
    const Actuator &longestControl = *boost::max_element(control_, cmpAc);
    return  (longestControl.start + longestControl.lasts);
}
//---------------------------------------------------------
double RoboMoves::Record::ratioUsedMusclesCount() const
{
    /* Количество задействованных мышц */
    ///double muscles_count = 0.;
    ///for (muscle_t m = 0; m < RoboI::musclesMaxCount; ++m)
    ///    if (br::find(control_, m) != control_.end())
    ///        ++muscles_count;
    ///return  (1. / muscles_count);
    return 0.;
}
double RoboMoves::Record::ratioTrajectoryBrakes() const
{
    /// TODO: ПЕРEЛОМЫ = остановки

    /// for ( auto m : mus )
    /// if ( Opn and Cls in controls )
    /// or start > start + last
    /// (1. / controlsCount) * /* Количество движений != ПЕРЕЛОМ */
    return 0.;
}
//------------------------------------------------------------------------------
tostream& RoboMoves::operator<<(tostream &s, const RoboMoves::Record &rec)
{
    s << rec.aim_ << std::endl;
    s << rec.move_begin_ << std::endl;
    s << rec.move_final_ << std::endl;
    s << rec.control_ << std::endl;
    s << rec.visited_.size() << _T(' ');
    for (auto p : rec.visited_)
        s << p << _T(' ');
    s << std::endl << std::endl;
    s << rec._lasts_step;
    // s << (_update_time - std::time(NULL)) << _update_traj;
    ////s << _error_distance;
    return s;
}
tistream& RoboMoves::operator>>(tistream &s, RoboMoves::Record &rec)
{
    size_t sz;
    s >> rec.aim_;
    s >> rec.move_begin_;
    s >> rec.move_final_;
    s >> rec.control_;
    s >> sz;
    for (auto i : boost::irange<size_t>(0, sz))
    {
        Point p;
        s >> p;
        rec.visited_.push_back(p);
    }
    s >> rec._lasts_step;
    // s >> _update_time >> _update_traj;
    // _update_time += std::time(NULL);
    rec.updateErrDistance(rec.move_final_);
    ////s >> _error_distance;
    return s;
}
//------------------------------------------------------------------------------
int64_t RoboMoves::getStrategy(const Robo::Control &controls)
{
    const auto k = int(std::ceil(std::log2(RoboI::musclesMaxCount))); // k = 3 = log2(8)
    if (controls.size() > 20) // 64 / 3 ~ 21
        CERROR("Too long control=" << controls.size() << " >20");
    int64_t n_strat = 0;
    int i = 0;
    for (auto &a : controls)
        n_strat += (int64_t(a.muscle) << (k*i++));
    return n_strat;
}
