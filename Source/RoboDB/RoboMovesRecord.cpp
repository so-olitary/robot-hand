﻿#include "RoboMovesRecord.h"
#include "RoboMovesStore.h"

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
void RoboMoves::Record::clear()
{
    aim_ = { 0.,0. };
    move_begin_ = { 0.,0. };
    move_final_ = { 0.,0. };
    control_.clear();
    visited_.clear();

    _strategy = -1;
    _lasts_step = 0;
    _error_distance = 0.;
    _update_time = 0;
    _update_traj = 0;
}
//---------------------------------------------------------
distance_t RoboMoves::Record::eleganceMove() const
{
    distance_t res = 0;
    /* Количество движений */
    res += distance_t(1) / getNControls();
    /* Суммарное время работы двигателей */
    res += ratioSumOfWorkingTime();
    /* Количество задействованных мышц */
    res += ratioUsedMusclesCount();
    /* max.отклонение от оптимальной траектории */
    res += ratioTrajectoryDivirgence();
    /* Длина траектории по сравнениею с дистанцией */
    res += ratioDistanceByTrajectory();
    /* Переломы в движении */
    res += ratioTrajectoryBrakes();
    return res;
}
//---------------------------------------------------------
distance_t RoboMoves::Record::ratioDistanceByTrajectory() const
{
    distance_t visited_distance = 0;
    for (auto curr = visited_.begin(), next = std::next(curr); next != visited_.end(); curr = next, ++next)
        visited_distance += bg::distance(*curr, *next);
    /* длина траектории по сравнениею с дистанцией */
    return (visited_distance > 0) ? (bg::distance(aim_, move_begin_) / visited_distance) : 1.;
}
distance_t RoboMoves::Record::ratioTrajectoryDivirgence() const
{
    using Line = boost::geometry::model::linestring<Point>;
    Line line{ move_begin_, aim };
    distance_t max_divirgence = 0.;
    /* max. отклонение от кратчайшего пути - прямой */
    br::for_each(visited_, [&line, &max_divirgence](const Point &p) {
        distance_t d = bg::distance(p, line);
        if (max_divirgence < d)
            max_divirgence = d;
    });
    return distance_t(1) / max_divirgence;
}
//---------------------------------------------------------
Robo::frames_t RoboMoves::Record::longestMusclesControl() const
{
    auto cmpAc = [](const Actuator &a, const Actuator &b) {
        return ((a.start + a.lasts) < (b.start + b.lasts));
    };
    const Actuator &longestControl = *boost::max_element(control_, cmpAc);
    return (longestControl.start + longestControl.lasts);
}
//---------------------------------------------------------
distance_t RoboMoves::Record::ratioSumOfWorkingTime() const
{
    frames_t sum = 0;
    br::for_each(control_, [&sum](const Actuator &a) { sum += a.lasts; });
    return distance_t(1) / sum;
}
distance_t RoboMoves::Record::ratioUsedMusclesCount() const
{
    muscle_t muscles = 0, max_m = 0;
    br::for_each(control_, [&muscles, &max_m](const Actuator &a) {
        max_m = std::max(max_m, a.muscle);
        muscles |= (1 << a.muscle);
    });
    /* Количество задействованных мышц */
    unsigned count = 0;
    for (muscle_t m = 0; m < max_m; ++m)
        count += (muscles & (1 << m)) ? 1 : 0;
    return distance_t(1) / count;
}
distance_t RoboMoves::Record::ratioTrajectoryBrakes() const
{
    unsigned count = 0;
    // резакая смена направления - ПЕРEЛОМЫ и остановки
    distance_t small = (2 * RoboI::minFrameMove);
    for (auto prev = visited_.begin(), curr = std::next(prev), next = std::next(curr);
         next != visited_.end();
         prev = curr, curr = next, ++next)
    {
        auto a = angle_degrees(*prev, *curr, *next);
        if (a < 120 || a > 240 || bg::distance(*curr, *next) < small)
            ++count;
    }
    return distance_t(1) / count;
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
