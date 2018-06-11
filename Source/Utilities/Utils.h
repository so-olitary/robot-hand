﻿#pragma once


namespace Utils
{
//-------------------------------------------------------------------------------
const double EPSILONT        = 1e-4;
const double EPSILONT_VISUAL = 1e-2;

//-------------------------------------------------------------------------------
template <typename Integer>
Integer  random(Integer max)
{ return  (max) ? (static_cast<Integer> (rand()) % (max)) : (max); }
template <typename Integer>
Integer  random(Integer min, Integer max)
{ return  (max) ? (static_cast<Integer> (rand()) % (max)+min) : (max); }

//-------------------------------------------------------------------------------
inline double  random(double max)
{ return  (static_cast<double> (rand()) / RAND_MAX) * max; }
inline double  random(double min, double max)
{ return  (static_cast<double> (rand()) / RAND_MAX) * (max - min) + min; }

//-------------------------------------------------------------------------------
template<typename INTEGER>
INTEGER interval_map(INTEGER input, std::pair<INTEGER, INTEGER> inv_in, std::pair<INTEGER, INTEGER> inv_out)
{ return (inv_out.second - inv_out.first) * (input - inv_in.first) / (inv_in.second - inv_in.first) + inv_out.first; }

//-------------------------------------------------------------------------------
tstring uni(const std::string&);
tstring uni(const std::wstring&);
//-------------------------------------------------------------------------------
tstring format(const TCHAR *fmt, ...);
//-------------------------------------------------------------------------------
}

//-------------------------------------------------------------------------------
tstring getLastErrorString();
tstring getCurrentTimeString(tstring format, std::time_t *the_time);
//-------------------------------------------------------------------------------
class ConfirmInput
{
    const TCHAR *_confirm;
public:
    ConfirmInput(const TCHAR *confirm) : _confirm(confirm) {}
    friend tistream& operator>>(tistream &s, const ConfirmInput &ci)
    {
        TCHAR c;
        for (const TCHAR* p = ci._confirm; *p; ++p)
        {
            if (std::isspace(*p))
                tistream::sentry k(s); // discard whitespace
            else if ((c = s.get()) != *p)
                s.setstate(std::ios::failbit); // stop extracting
        }
        return s;
    }
};
//-------------------------------------------------------------------------------