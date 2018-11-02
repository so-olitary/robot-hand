﻿#include "Robo.h"
#include "RoboControl.h"


const double Robo::RoboI::minFrameMove = Utils::EPSILONT;
//---------------------------------------------------------
Robo::Control::Control(const Robo::Actuator *a, size_t sz)
{
    if (sz > MAX_ACTUATORS)
        throw std::logic_error("Control: Too much actuators for Control!");

    for (actuals = 0; actuals < sz; ++actuals)
        actuators[actuals] = a[actuals];
    _validated = false;
}

//---------------------------------------------------------
void Robo::Control::append(const Robo::Actuator &a)
{
    auto f = br::find(actuators, a);
    if (f != std::end(actuators))
        throw std::logic_error("append: duplicate");

    if (actuals >= MAX_ACTUATORS)
        throw std::logic_error("append: Too much actuators for Control!");
    // actuals is INDEX TO INSERT and LENGTH
    if (actuals == 0 || actuators[actuals - 1].start <= a.start)
    {
        actuators[actuals++] = a;
        return;
    }
    // sorted insert
    for (size_t i = 0; i < actuals; ++i)
        if (actuators[i] >= a)
        {
            memmove(&actuators[i + 1], &actuators[i], (actuals + 1 - i) * sizeof(Actuator));
            actuators[i] = a;
            ++actuals;
            return;
        }
    _validated = false;
}

//---------------------------------------------------------
void Robo::Control::remove(size_t i)
{
    if (!actuals)
        throw std::logic_error("remove: Controls are empty!");

    actuals--;
    for (size_t j = i; j < actuals; ++j)
        actuators[j] = actuators[j + 1];
    actuators[actuals] = { MInvalid, 0, 0 };
    _validated = false;
}

//---------------------------------------------------------
void Robo::Control::pop_back()
{
    if (!actuals)
        throw std::logic_error("pop_back: Controls are empty!");
    actuals--;
    actuators[actuals] = { MInvalid, 0, 0 };
    _validated = false;
}

//---------------------------------------------------------
bool Robo::Control::operator==(const Robo::Control &c) const
{
    if (this == &c)
        return true;
    if (actuals != c.actuals)
        return false;
    for (size_t i = 0; i < actuals; ++i)
        if (actuators[i] != c.actuators[i])
            return false;
    return true;
}

//---------------------------------------------------------
void Robo::Control::removeStartPause()
{
    Robo::frames_t start_time = actuators[0].start;
    if (start_time > 0)
        for (size_t i = 0; i < actuals; ++i)
            actuators[i].start -= start_time;
    _validated = false;
}

//---------------------------------------------------------
bool intersect(const Robo::Actuator &a1, const Robo::Actuator &a2)
{
    return (a1.start >= a2.start && a1.start <= (a2.start + a2.lasts)) ||
           (a1.lasts >= a2.start && a1.lasts <= (a2.start + a2.lasts)) ||
           (a2.start >= a1.start && a2.start <= (a1.start + a1.lasts)) ||
           (a2.lasts >= a1.start && a2.lasts <= (a1.start + a1.lasts));
}

//----------------------------------------------------
bool Robo::Control::intersectMusclesOpposites() const
{
    if (actuals <= 1)
        return true;
    for (auto iti = this->begin(); iti != this->end(); ++iti)
        for (auto itj = std::next(iti); itj != this->end(); ++itj)
            if (itj->start > (iti->start + iti->lasts))
                break; // т.к. отсортированы по возрастанию start
            // есть противоположные мышцы и есть перекрытие по времени
            else if (RoboI::muscleOpposite(iti->muscle) == itj->muscle && intersect(*iti, *itj))
                return false;
    return true;
}

//---------------------------------------------------------
void Robo::Control::fillRandom(Robo::muscle_t n_muscles, const MaxMuscleCount &muscleMaxLasts,
                               Robo::frames_t lasts_min, unsigned min_n_moves, unsigned max_n_moves, bool simul)
{
    if (min_n_moves <= 0 || max_n_moves <= 0 || max_n_moves < min_n_moves)
        throw std::logic_error("fillRandom: Incorrect Min Max n_moves");

    unsigned moves_count = Utils::random(min_n_moves, max_n_moves);
    if (moves_count >= MAX_ACTUATORS)
        throw std::logic_error("fillRandom: Too much actuators for Control!");

    clear();
    Robo::Actuator a{};
    for (unsigned mv = 0; mv < moves_count; ++mv)
    {
        a.muscle = Utils::random(n_muscles);
        while (!a.lasts)
            a.lasts = Utils::random(lasts_min, muscleMaxLasts(a.muscle));
        actuators[actuals++] = a; // append to end NOT SORTED

        if (!simul)
            a.start += (a.lasts + 1);
        else
            a.start = Utils::random<frames_t>(0, a.lasts + 1);
    }
    br::sort(*this); // then SORT
    // remove duplicates and combine intersects
    for (auto i = 0u; i < (this->size() - 1u); ++i)
        for (auto j = i + 1u; j < this->size();)
            if (actuators[i].muscle == actuators[j].muscle && 
                intersect(actuators[i], actuators[j]))
                {
                    actuators[i].lasts = std::max(actuators[i].start + actuators[i].lasts,
                                                  actuators[j].start + actuators[j].lasts);
                    actuators[i].start = std::min(actuators[i].start, actuators[j].start);
                    actuators[i].lasts -= actuators[i].start;
                    std::memmove(&actuators[j], &actuators[j + 1u], actuals - j);
                    --actuals;
                    actuators[actuals] = { MInvalid, 0, 0};
                }
                else
                    ++j;
    _validated = true;
}

//---------------------------------------------------------
auto Robo::Control::begin()       -> decltype(boost::begin(actuators)) { return boost::begin(actuators); }
auto Robo::Control::begin() const -> decltype(boost::begin(actuators)) { return boost::begin(actuators); }
auto Robo::Control::end()         -> decltype(boost::end(actuators))   { return boost::begin(actuators) + actuals; } // ?? std::advance()
auto Robo::Control::end()   const -> decltype(boost::end(actuators))   { return boost::begin(actuators) + actuals; } // ?? std::advance()

//---------------------------------------------------------
void Robo::Control::validated(Robo::muscle_t n_muscles) const
{
    if (_validated) return;
    /* Что-то должно двигаться, иначе беск.цикл */
    if (!actuals)
        throw std::logic_error("validated: Controls are empty!\r\n" + this->str());
    /* Управление должно быть отсортировано по времени запуска двигателя */
    frames_t start = 0;
    auto not_sorted = [&start](const Actuator &a) { 
        bool res = (start > a.start);
        start = a.start;
        return res;
    };
    if (/*actuators[0].start != 0 ||*/ ba::any_of(*this, not_sorted))
        throw std::logic_error("validated: Controls are not sorted!\r\n" + this->str());
    /* Исключить незадействованные двигатели */
    if (ba::any_of(*this, [n_muscles](const Actuator &a) { return (!a.lasts) || (a.muscle >= n_muscles); }))
        throw std::logic_error("validated: Controls have UNUSED or INVALID muscles!\r\n" + this->str());
    _validated = true;
}

//---------------------------------------------------------
bool Robo::Control::validate(Robo::muscle_t n_muscles) const
{
    if (_validated) return true;
    frames_t start = 0; // actuators[0].start == 0
    auto is_invalid = [n_muscles, &start](const Actuator &a) {
        bool res = (start > a.start);
        start = a.start;
        return res || (!a.lasts) || (a.muscle >= n_muscles);
    };
    return _validated = (actuals > 0 && ba::none_of(*this, is_invalid));
}


//---------------------------------------------------------
//void  Hand::recursiveControlsAppend(muscle_t muscles, joint_t joints,
//                                    size_t cur_deep, size_t max_deep)
//{
//    for (auto j : joints_)
//    {
//        if (!(j & joints) && (j > joints))
//        {
//            if (cur_deep == max_deep)
//            {
//                controls.push_back(muscles | muscleByJoint(j, true));
//                controls.push_back(muscles | muscleByJoint(j, false));
//            }
//            else
//            {
//                recursiveControlsAppend(muscles | muscleByJoint(j, true), j | joints, cur_deep + 1U, max_deep);
//                recursiveControlsAppend(muscles | muscleByJoint(j, false), j | joints, cur_deep + 1U, max_deep);
//            } // end else
//        } // end if
//    } // end for
//};
//void  Hand::createControls()
//{
//    for (auto m : muscles_)
//    { controls.push_back(m); }
//
//    /* 1 << 0,1,2,3,4,5,6,7 */
//    /* Impossible (0 & 1) (2 & 3) (4 & 5) (6 & 7) */
//
//    size_t    maxSimultMoves = joints_.size();
//    for (size_t SimultMoves = 1U; SimultMoves < maxSimultMoves; ++SimultMoves)
//        recursiveControlsAppend(Hand::EmptyMov, Hand::Empty, 0U, SimultMoves);
//}

//---------------------------------------------------------
tostream& Robo::operator<<(tostream &s, const Robo::Actuator &a)
{
    return s << _T("{") << uint32_t(a.muscle)
             << _T(",") << a.start
             << _T(",") << a.lasts
             << _T("}");
}

//---------------------------------------------------------
tistream& Robo::operator>>(tistream &s, Robo::Actuator &a)
{ 
    uint32_t m;
    s >> ConfirmInput(_T("{")) >> m
      >> ConfirmInput(_T(",")) >> a.start
      >> ConfirmInput(_T(",")) >> a.lasts
      >> ConfirmInput(_T("}"));
    a.muscle = m;
    return s;
}

//---------------------------------------------------------
tostream& Robo::operator<<(tostream &s, const Robo::Control &controls)
{
    s << _T("c[");
    for (const Robo::Actuator &a : controls)
        s << a << (((&a - &controls.actuators[0]) == (controls.actuals - 1)) ? _T("") : _T(","));
    s << _T("]");
    return s;
}
//---------------------------------------------------------
tistream& Robo::operator>>(tistream &s, Robo::Control &controls)
{
    s >> ConfirmInput(_T("c["));
    {
        Robo::Actuator a;
        s >> a >> ConfirmInput(_T(","));
        controls.append(a);
    } while (s.fail());

    s.setstate(std::ios::goodbit);
    s >> ConfirmInput(_T("]"));
    return s;
}
//---------------------------------------------------------
std::ostream& Robo::Control::stream(std::ostream &s) const
{
    s << "c[";
    for (const auto &a : *this)
    {
        a.stream(s);
        s << (((&a - &actuators[0]) == (actuals - 1)) ? "" : ",");
    }
    s << "]";
    return s;
}
//---------------------------------------------------------
