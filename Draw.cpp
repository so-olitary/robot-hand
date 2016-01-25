﻿#include "StdAfx.h"
#include "Draw.h"
//------------------------------------------------------------------------------
void  draw_decards_coordinates (HDC hdc)
{ uint_t i = 90U;
	//---Ox---
  MoveToEx (hdc, Tx (-1.000), Ty ( 0.000), NULL);
  LineTo   (hdc, Tx ( 1.000), Ty ( 0.000));
  LineTo   (hdc, Tx ( 0.970), Ty ( 0.010));
  LineTo   (hdc, Tx ( 0.990), Ty ( 0.000));
  LineTo   (hdc, Tx ( 0.970), Ty (-0.010));
  LineTo   (hdc, Tx ( 1.000), Ty ( 0.000));
  //---Oy---													 
  MoveToEx (hdc, Tx ( 0.000), Ty (-1.000), NULL);
  LineTo   (hdc, Tx ( 0.000), Ty ( 1.000));
  LineTo   (hdc, Tx (-0.005), Ty ( 0.960));
  LineTo   (hdc, Tx ( 0.000), Ty ( 0.990));
  LineTo   (hdc, Tx ( 0.008), Ty ( 0.960));
  LineTo   (hdc, Tx ( 0.000), Ty ( 1.000));
	//--------
	for(i=0; i<19; i++)
	{
		 MoveToEx (hdc, Tx (-0.90 + 0.1*i), Ty (-0.01        ), NULL);
		 LineTo   (hdc, Tx (-0.90 + 0.1*i), Ty ( 0.01        ) );
		 MoveToEx (hdc, Tx (-0.01        ), Ty (-0.90 + 0.1*i), NULL);
		 LineTo   (hdc, Tx ( 0.01        ), Ty (-0.90 + 0.1*i) );
	}
}
//------------------------------------------------------------------------------
//#include <gdiplus.h>
/* Drawing the trajectory, where we got ... */
void  draw_trajectory (HDC hdc, std::list<Point> &trajectory, HPEN hPen)
{
  if ( !trajectory.empty () )
  {
    HPEN hPen_old = (HPEN) SelectObject (hdc, hPen);
    //------------------------------------------------------------------
    Point &p = trajectory.front ();
    MoveToEx (hdc, Tx (p.x), Ty (p.y), NULL);

    for ( auto p : trajectory )
    { LineTo (hdc, Tx (p.x), Ty (p.y)); }
    //------------------------------------------------------------------
    // отменяем ручку
    SelectObject (hdc, hPen_old);
  }
}
//------------------------------------------------------------------------------

