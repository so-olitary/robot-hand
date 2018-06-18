#pragma once

#include "Point.h"
#include "RoboMovesTarget.h"
#include "RoboMovesStore.h"
#include "RoboPosTour.h"

namespace RoboPos
{
class TourEvo : public RoboPos::TourI
{
protected:
    //const Robo::frames_t big_lasts = std::min(min_lasts * 1000, Robo::musclesMaxLasts(_robo));
    const double side = 0.001;
    
    Point _base_pos{};

    const TargetI &_t;
    double _reached_dist = 0.000001;
    double _oppo_penalty = 1.;
    double _prev_dist_add = Robo::RoboI::minFrameMove;

    Robo::frames_t _max_ncontrols = 10;
    Robo::frames_t _step_back = 4;

    Robo::frames_t minLasts();

    double reached_less(double dist) const { return (dist - _reached_dist / 2); }

    bool runNestedReset(IN const Robo::Control&, IN OUT Point &hit);

    bool runNestedPreMove(IN const Robo::Control&, IN Robo::frames_t max_frames, OUT Point &hit);

    bool runNestedStop(IN const Robo::bitset_t &muscles, IN bool stop);
    bool runNestedStop(IN const Robo::Control &controls, IN bool stop);

    bool runNestedStep(IN const Robo::bitset_t &muscles, IN Robo::frames_t lasts, OUT Point &hit);
    bool runNestedStep(IN const Robo::Control&, OUT Point &hit);

    bool runNestedForMuscle(Robo::joint_t, Robo::Control&, Point &robo_hit);
    bool runNestedForMuscleSteps(Robo::joint_t, Robo::Control&, Point&robo_hit);

    bool containStore(const Robo::Control &controls, Robo::joint_t muscles,
                      Robo::frames_t frame, Robo::frames_t lasts);

    bool stepBack(Robo::Control &controls, Robo::Control &controls_prev, distance_t curr_best_dist);

public:
    TourEvo(RoboMoves::Store &store, Robo::RoboI &robo, const TargetI &target);
    void setParams(double reached_dist, double oppo_penalty)
    {
        _reached_dist = reached_dist;
        _oppo_penalty = oppo_penalty;
    }
};

class TourEvoSteps : public TourEvo
{
protected:
    bool runNestedForStep(const Robo::RoboI::bitwise &muscles, Point &robo_hit);
    bool runNestedForMuscle(Robo::joint_t, Robo::Control&, Point &robo_hit);
    bool compansateOverHit(Robo::Control &controls, const Point &goal);

public:
    TourEvoSteps(RoboMoves::Store &store, Robo::RoboI &robo, const TargetI &target);
};

}
