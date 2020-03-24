﻿#pragma once

#include "RoboPhysics.h"

namespace Robo {
class EnvEdgesTank;
namespace Mobile {

#define TANK_VER 2
#define TANK_DEBUG

class Tank : public RoboPhysics
{
public:
    struct JointInput;
    enum /*class*/ Muscle : uint8_t
    {
        LTrackFrw = 0, // Frw = forward
        LTrackBck = 1, // Bck = backward
        RTrackFrw = 2,
        RTrackBck = 3,
        MCount = 4,
        MInvalid = 5
    };
    enum /*class*/ Joint : uint8_t
    {
        LTrack = 0,
        RTrack = 1,
        JCount = 2,
        Center = 2,
        JInvalid = 3
    };

    Tank::Muscle MofJ(Tank::Joint j, bool frwd) const { return Tank::Muscle(muscle_t(j) * musclesPerJoint + !frwd); }
    Tank::Joint  JofM(Tank::Muscle m)           const { return Tank::Joint(joint_t(m) / musclesPerJoint); }
    Tank::Muscle M(muscle_t m) const { return (m >= muscles) ? Tank::Muscle::MInvalid : params.musclesUsed[m]; }
    Tank::Joint  J(joint_t j)  const { return (j >= joints)   ? Tank::Joint::JInvalid  : params.jointsUsed[j]; }

protected:
    static const size_t muscles = (size_t)(Tank::Muscle::MCount);
    static const size_t joints = (size_t)(Tank::Joint::JCount);

    struct Params
    {
        std::array<Tank::Muscle, muscles> musclesUsed{};
        std::array<Tank::Joint, joints> jointsUsed{};
        //--- draw constants
        double   trackWidth{};
        double  trackHeight{};
        double   bodyHeight{};
        double centerRadius{};

        Params(const JointsInputsPtrs&, const Tank&);
    };
    const Params params;

#ifdef TANK_DEBUG
    Point center_;
    double r1_, r2_;
#endif
    void realMove();

    distance_t prismatic_factor(joint_t) const { return 1.; }
    
public:
    Tank(const Point &baseCenter, const JointsInputsPtrs &joints);

    frames_t muscleMaxLasts(muscle_t muscle) const;
    frames_t muscleMaxLasts(const Robo::Control &control) const;

    void getWorkSpace(OUT Trajectory &workSpace);
    void draw(IN HDC hdc, IN HPEN hPen, IN HBRUSH hBrush) const;
    int specPoint() const { return static_cast<int>(Joint::Center); }
    
    void resetJoint(IN joint_t);
    void setJoints(IN const Robo::JointsOpenPercent&);
    
    const Point& position() const;

    tstring getName() const { return Tank::name(); }
    static tstring name() { return _T("Tank-v2"); }
    static std::shared_ptr<RoboI> make(const tstring &type, tptree &root);
    friend class Robo::EnvEdgesTank;
};

} // end namespace Mobile
} // end namespace Robo
