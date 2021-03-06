﻿#include "StdAfx.h"
#include "WindowData.h"
#include "Draw.h"
#include "Position.h"

using namespace std;
using namespace HandMoves;

bool zoom = false;
//------------------------------------------------------------------------------
win_point*  WindowSize (void)
{ static win_point Width_Height;
  return &Width_Height;
}
void     SetWindowSize (int Width_, int Height_)
{ WindowSize ()->x = Width_;
  WindowSize ()->y = Height_;
}

/* normal */
int  T2x (double logic_x)
{ return  (int) (MARGIN + ( 0.5) * (logic_x + 1.) * (WindowSize ()->x - 2. * MARGIN)); }
int  T2y (double logic_y)
{ return  (int) (MARGIN + (-0.5) * (logic_y - 1.) * (WindowSize ()->y - 2. * MARGIN)); }

/* zoom */
int  T1x (double logic_x)
{ return  (int) (MARGIN + ( 1.) * (logic_x + 0.5) * (WindowSize ()->x - 2. * MARGIN)); }
int  T1y (double logic_y)
{ return  (int) (MARGIN + (-1.) * (logic_y - 0.0) * (WindowSize ()->y - 2. * MARGIN)); }

int  Tx  (double logic_x)
{ return  (zoom) ? T1x (logic_x) : T2x (logic_x); }
int  Ty  (double logic_y)
{ return  (zoom) ? T1y (logic_y) : T2y (logic_y); }

Point  logic_coord (win_point* coord)
{
  Point p;
  if ( zoom )
  {
    p.x = ((coord->x - MARGIN) / (( 1.0) * (WindowSize ()->x - 2. * MARGIN))) - 0.5;
    p.y = ((coord->y - MARGIN) / ((-1.0) * (WindowSize ()->y - 2. * MARGIN))) + 0.0;
  }
  else
  {
    p.x = ((coord->x - MARGIN) / (( 0.5) * (WindowSize ()->x - 2. * MARGIN))) - 1.;
    p.y = ((coord->y - MARGIN) / ((-0.5) * (WindowSize ()->y - 2. * MARGIN))) + 1.;
  }
  return p;
}
//-------------------------------------------------------------------------------
/* Create a string with last error message */
tstring  GetLastErrorToString ()
{
  DWORD  error = GetLastError ();
  if ( error )
  {
    LPVOID lpMsgBuf;
    DWORD bufLen = FormatMessageW ( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                    // FORMAT_MESSAGE_IGNORE_INSERTS |
                                    FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL,
                                    error,
                                    MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                                    (LPTSTR) &lpMsgBuf,
                                    0, NULL);
    if ( bufLen )
    {
      LPCSTR lpMsgStr = (LPCSTR) lpMsgBuf;
      tstring result (lpMsgStr, lpMsgStr + bufLen);

      LocalFree (lpMsgBuf);
      return result;
    }
  }
  return  tstring ();
}
//-------------------------------------------------------------------------------
void OnWindowCreate (HWND &hWnd, RECT &myRect,
                     HWND &hLabCanv, HWND &hLabHelp,
                     HWND &hLabMAim, HWND &hLabTest,
                     HWND &hLabStat, LabelsPositions &lp)
{
  RECT Rect;

  GetClientRect (hWnd, &Rect);
  myRect.left = Rect.left;
  myRect.top = Rect.top;
  myRect.bottom = Rect.bottom;
  myRect.right = Rect.left + (Rect.bottom - Rect.top);

  lp.LabelsLeft    = Rect.bottom - Rect.top;
  lp.LabelsWidth   = Rect.right - (Rect.bottom - Rect.top);

  lp.LabCanvTop    = Rect.top;
  lp.LabCanvHeight = 24U;
  lp.LabHelpTop    = Rect.top + 25U;
  lp.LabHelpHeight = (Rect.bottom - Rect.top) * 2U / 3U - 1U;
  lp.LabMAimTop    = Rect.top +  25U + (Rect.bottom - Rect.top) * 2U / 3U;
  lp.LabMAimHeight = 49U;
  lp.LabTestTop    = Rect.top +  75U + (Rect.bottom - Rect.top) * 2U / 3U;
  lp.LabTestHeight = 49U;
  lp.LabStatTop    = Rect.top + 125U + (Rect.bottom - Rect.top) * 2U / 3U;
  lp.LabStatHeight = Rect.bottom - ((Rect.bottom - Rect.top) * 2U / 3U - 125U);

  // Create a Static Label control
  hLabCanv = CreateWindow (_T ("STATIC"),            /* The name of the static control's class */
                           _T ("Canvas  "),                                    /* Label's Text */
                           WS_CHILD | WS_VISIBLE | SS_RIGHT | WS_BORDER, /* Styles (continued) */
                           (int) lp.LabelsLeft,                              /* X co-ordinates */
                           (int) lp.LabCanvTop,                              /* Y co-ordinates */
                           (int) lp.LabelsWidth,                                      /* Width */
                           (int) lp.LabCanvHeight,                                   /* Height */
                           hWnd,                                                /* Parent HWND */
                           (HMENU) IDL_CANVAS,                               /* The Label's ID */
                           NULL,                              /* The HINSTANCE of your program */
                           NULL);                                /* Parameters for main window */
  if ( !hLabCanv )
    MessageBox (hWnd, _T ("Could not create hLabCanv."), _T ("Error"), MB_OK | MB_ICONERROR);

  // Create a Static Label control
  hLabHelp = CreateWindow (_T ("STATIC"),            /* The name of the static control's class */
                           _T ("Help"),                                        /* Label's Text */
                           WS_CHILD | WS_VISIBLE | SS_LEFT  | WS_BORDER, /* Styles (continued) */
                           (int) lp.LabelsLeft,                              /* X co-ordinates */
                           (int) lp.LabHelpTop,                              /* Y co-ordinates */
                           (int) lp.LabelsWidth,                                      /* Width */
                           (int) lp.LabHelpHeight,                                   /* Height */
                           hWnd,                                                /* Parent HWND */
                           (HMENU) IDL_HELP,                                 /* The Label's ID */
                           NULL,                              /* The HINSTANCE of your program */
                           NULL);                                /* Parameters for main window */
  if ( !hLabHelp )
    MessageBox (hWnd, _T ("Could not create hLabHelp."), _T ("Error"), MB_OK | MB_ICONERROR);

  // Create a Static Label control
  hLabMAim = CreateWindow (_T ("STATIC"),            /* The name of the static control's class */
                           _T (" "),                                           /* Label's Text */
                           WS_CHILD | WS_VISIBLE | SS_RIGHT | WS_BORDER, /* Styles (continued) */
                           (int) lp.LabelsLeft,                              /* X co-ordinates */
                           (int) lp.LabTestTop,                              /* Y co-ordinates */
                           (int) lp.LabelsWidth,                                      /* Width */
                           (int) lp.LabTestHeight,                                   /* Height */
                           hWnd,                                                /* Parent HWND */
                           (HMENU) IDL_MAIM,                                 /* The Label's ID */
                           NULL,                              /* The HINSTANCE of your program */
                           NULL);                                /* Parameters for main window */
  if ( !hLabMAim )
    MessageBox (hWnd, _T ("Could not create hLabMAim."), _T ("Error"), MB_OK | MB_ICONERROR);
  
  // Create a Static Label control
  hLabTest = CreateWindow (_T ("STATIC"),            /* The name of the static control's class */
                           _T (" "),                                           /* Label's Text */
                           WS_CHILD | WS_VISIBLE | SS_RIGHT | WS_BORDER, /* Styles (continued) */
                           (int) lp.LabelsLeft,                              /* X co-ordinates */
                           (int) lp.LabStatTop,                              /* Y co-ordinates */
                           (int) lp.LabelsWidth,                                      /* Width */
                           (int) lp.LabStatHeight,                                   /* Height */
                           hWnd,                                                /* Parent HWND */
                           (HMENU) IDL_TEST,                                 /* The Label's ID */
                           NULL,                              /* The HINSTANCE of your program */
                           NULL);                                /* Parameters for main window */
  if ( !hLabTest )
    MessageBox (hWnd, _T ("Could not create hLabTest."), _T ("Error"), MB_OK | MB_ICONERROR);
  
  // Create a Static Label control
  hLabStat = CreateWindow (_T ("STATIC"),            /* The name of the static control's class */
                           _T (" "),                                           /* Label's Text */
                           WS_CHILD | WS_VISIBLE | SS_RIGHT | WS_BORDER, /* Styles (continued) */
                           (int) lp.LabelsLeft,                              /* X co-ordinates */
                           (int) lp.LabCanvTop,                              /* Y co-ordinates */
                           (int) lp.LabelsWidth,                                      /* Width */
                           (int) lp.LabCanvHeight,                                   /* Height */
                           hWnd,                                                /* Parent HWND */
                           (HMENU) IDL_STAT,                                 /* The Label's ID */
                           NULL,                              /* The HINSTANCE of your program */
                           NULL);                                /* Parameters for main window */
  if ( !hLabStat )
    MessageBox (hWnd, _T ("Could not create hLabStat."), _T ("Error"), MB_OK | MB_ICONERROR);

  // Generate the help string
  tstringstream ss;
  ss << _T ("  Клавиши управления:  \r\r") // "Enter - авто-тест  \r")
     << _T ("  Ctrl+O - OpenFile  |  Ctrl+S - SaveFile  |  Esc - выход  \r\r")
     << _T ("  R - сбросить состояние руки  \r\r")
     << _T ("  Z - двинуть ключицей вправо  \r  X - сомкнуть плечо  \r")
     << _T ("  С - сомкнуть локоть  \r  V - сомкнуть ладонь  \r")
     << _T ("  A - двинуть ключицей влево  \r  S - раскрыть плечо  \r")
     << _T ("  D - раскрыть локоть  \r  F - раскрыть ладонь  \r")
     << _T ("  Повторное нажатие на кнопку во время движения  \r")
     << _T ("  останавливает соответствующее движение.  \r\r")
     << _T ("  Q - показать все конечные точки в БД  \r")
     << _T ("  W - нарисовать рабочую область руки  \r")
     << _T ("  E - прервать работу алгоритма  \r")
     << _T ("  T - нарисовать случайную траекторию  \r")
     << _T ("  Y - приблизить, показать только мишень  \r")
     << _T ("  U - посчитать непокрытые точки мишени  \r")
     << _T ("  H - показать  непокрытые точки мишени  \r")
     << _T ("  G - показать масштаб и размеры  \r\r")
     << _T ("  O - Random Test,   P - Cover Test  \r")
     << _T ("  1 - STAGE,  2 - STAGE,  3 - STAGE  \r\r")
     << _T ("  \r\r");
  // << _T ("Для выбора цели отрисовки  \r")
  // << _T ("M + !no!/%2u + Enter,  \rN + !no!/%2u + Enter   \r")
  // << _T (", где 0 <= !no! - номер строки/столбца"),

  // Setting the Label's text
  SendMessage (hLabHelp,         /* Label   */
               WM_SETTEXT,       /* Message */
               (WPARAM) NULL,    /* Unused  */
               (LPARAM) ss.str ().c_str ());

  SetTimer (hWnd,                   /* Handle to main window */
            IDT_TIMER_VISION,       /* Timer identifier      */
            30,                     /* 1/10-second interval  */
            (TIMERPROC) NULL);      /* No Timer callback     */
}

void OnWindowSize (HWND &hWnd, RECT &myRect,
                   HWND &hLabCanv, HWND &hLabHelp,
                   HWND &hLabMAim, HWND &hLabTest,
                   HWND &hLabStat, LabelsPositions &lp)
{
  RECT Rect;

  GetClientRect (hWnd, &Rect);
  myRect.left = Rect.left;
  myRect.top = Rect.top;
  myRect.bottom = Rect.bottom;
  myRect.right = Rect.left + (Rect.bottom - Rect.top);

  SetWindowSize ( /* Rect.right - Rect.left */
                 Rect.bottom - Rect.top,
                 Rect.bottom - Rect.top);

  lp.LabelsLeft = Rect.bottom - Rect.top;
  lp.LabelsWidth = Rect.right - (Rect.bottom - Rect.top);

  lp.LabCanvTop = Rect.top;
  lp.LabCanvHeight = 24U;
  lp.LabHelpTop = Rect.top + 25U;
  lp.LabHelpHeight = (Rect.bottom - Rect.top) * 2U / 3U - 1U;
  lp.LabMAimTop = Rect.top + 25U + (Rect.bottom - Rect.top) * 2U / 3U;
  lp.LabMAimHeight = 49U;
  lp.LabTestTop = Rect.top + 75U + (Rect.bottom - Rect.top) * 2U / 3U;
  lp.LabTestHeight = 49U;
  lp.LabStatTop = Rect.top + 125U + (Rect.bottom - Rect.top) * 2U / 3U;
  lp.LabStatHeight = Rect.bottom - ((Rect.bottom - Rect.top) * 2U / 3U - 125U);

  /* Set position noCanvas label */
  SetWindowPos (hLabCanv, NULL,
                (int) lp.LabelsLeft,     /* X co-ordinates */
                (int) lp.LabCanvTop,     /* Y co-ordinates */
                (int) lp.LabelsWidth,             /* Width */
                (int) lp.LabCanvHeight,          /* Height */
                (UINT) NULL);   
  /* Set position help label */ 
  SetWindowPos (hLabHelp, NULL, 
                (int) lp.LabelsLeft,     /* X co-ordinates */
                (int) lp.LabHelpTop,     /* Y co-ordinates */
                (int) lp.LabelsWidth,             /* Width */
                (int) lp.LabHelpHeight,          /* Height */
                (UINT) NULL);   
  /* Set position help label */ 
  SetWindowPos (hLabMAim, NULL, 
                (int) lp.LabelsLeft,     /* X co-ordinates */
                (int) lp.LabMAimTop,     /* Y co-ordinates */
                (int) lp.LabelsWidth,             /* Width */
                (int) lp.LabMAimHeight,          /* Height */
                (UINT) NULL);
  /* Set position help label */
  SetWindowPos (hLabTest, NULL, 
                (int) lp.LabelsLeft,     /* X co-ordinates */
                (int) lp.LabTestTop,     /* Y co-ordinates */
                (int) lp.LabelsWidth,             /* Width */
                (int) lp.LabTestHeight,          /* Height */
                (UINT) NULL);   
  /* Set position help label */ 
  SetWindowPos (hLabStat, NULL, 
                (int) lp.LabelsLeft,     /* X co-ordinates */
                (int) lp.LabStatTop,     /* Y co-ordinates */
                (int) lp.LabelsWidth,             /* Width */
                (int) lp.LabStatHeight,          /* Height */
                (UINT) NULL);   
}

void OnWindowPaint (HWND &hWnd, RECT &myRect,
                    MyWindowData &wd)
{
  PAINTSTRUCT ps = {};
  
  HDC  hdc = BeginPaint (hWnd, &ps);
  //-------------------------------------------
  if ( wd.hStaticBitmapChanged && !wd.testing )
  {
    /* Создание ещё одного теневого контекста
     * для отрисовки неизменной и
     * (в некоторых случаях ресурсоёмкой)
     * части картинки единожды.
     */
    if ( !wd.hStaticDC )
    {
      wd.hStaticDC = CreateCompatibleDC (hdc);
      if ( !wd.hStaticDC )
      {
        MessageBox (hWnd, GetLastErrorToString ().c_str (),
                    _T ("ERROR"), MB_OK | MB_ICONERROR);
      }
    }

    /* Удаляем старый объект */
    if ( wd.hStaticBitmap )
    {
      DeleteObject (wd.hStaticBitmap);
      wd.hStaticBitmap = NULL;
    }
    /* Создаём новый растровый холст */
    wd.hStaticBitmap = CreateCompatibleBitmap (hdc, myRect.right  - myRect.left,
                                                    myRect.bottom - myRect.top);
    if ( !wd.hStaticBitmap )
    {
      MessageBox (hWnd, GetLastErrorToString ().c_str (),
                  _T ("ERROR"), MB_OK | MB_ICONERROR);
    }
    SelectObject (wd.hStaticDC, wd.hStaticBitmap);

    /* Рисуем всё заново */
    //======================================
    /* Закраска фона рабочей области */
    FillRect (wd.hStaticDC, &myRect, wd.hBrush_back);
    /* set transparent brush to fill shapes */
    SelectObject (wd.hStaticDC, wd.hBrush_null);
    SetBkMode    (wd.hStaticDC, TRANSPARENT);
    //-------------------------------------
    OnPaintStaticBckGrnd (wd.hStaticDC, wd);
    //-------------------------------------
    /* Здесь рисуем на контексте hCmpDC */
    OnPaintStaticFigures (wd.hStaticDC, wd);
    wd.hStaticBitmapChanged = false;
    //======================================
  }
  //-------------------------------------
  /* Создание теневого контекста для двойной буфферизации */
  HDC   hCmpDC = CreateCompatibleDC (hdc);
  if ( !hCmpDC )
  { MessageBox (hWnd, GetLastErrorToString ().c_str (),
                _T ("ERROR"), MB_OK | MB_ICONERROR);
  }

  HBITMAP  hBmp = CreateCompatibleBitmap (hdc, myRect.right  - myRect.left,
                                               myRect.bottom - myRect.top);
  if ( !hBmp )
  { MessageBox (hWnd, GetLastErrorToString ().c_str (),
                _T ("ERROR"), MB_OK | MB_ICONERROR);
  }
  SelectObject (hCmpDC, hBmp);
  //-------------------------------------
  if ( !wd.testing )
  {
    SetStretchBltMode (hCmpDC, COLORONCOLOR);
    BitBlt (hCmpDC, 0, 0,
            myRect.right - myRect.left,
            myRect.bottom - myRect.top,
            wd.hStaticDC, 0, 0,
            SRCCOPY);
    //-------------------------------------
    /* set transparent brush to fill shapes */
    SelectObject (hCmpDC, wd.hBrush_null);
    SetBkMode (hCmpDC, TRANSPARENT);
  }
  else
  {
    /* Рисуем всё заново */
    //======================================
    /* Закраска фона рабочей области */
    FillRect (hCmpDC, &myRect, wd.hBrush_back);
    /* set transparent brush to fill shapes */
    SelectObject (hCmpDC, wd.hBrush_null);
    SetBkMode (hCmpDC, TRANSPARENT);
    //-------------------------------------
    OnPaintStaticBckGrnd (hCmpDC, wd);
    //-------------------------------------
  }
  //-------------------------------------
  /* Здесь рисуем на контексте hCmpDC */
  OnPainDynamicFigures (hCmpDC, wd);
  //-------------------------------------
  /* Копируем изображение из теневого контекста на экран */
  SetStretchBltMode (hdc, COLORONCOLOR);
  BitBlt (hdc, 0, 0,
          myRect.right  - myRect.left,
          myRect.bottom - myRect.top,
          hCmpDC, 0, 0,
          SRCCOPY);

  /* Удаляем ненужные системные объекты */
  DeleteDC (hCmpDC);
  DeleteObject (hBmp);
  //---------------------------------------------
  EndPaint (hWnd, &ps);
}
//-------------------------------------------------------------------------------
void OnWindowKeyDown (HWND &hWnd, RECT &myRect,
                      WPARAM wParam, LPARAM lparam,
                      MyWindowData &wd)
{
  char symbol = (char) (wParam);
  switch ( symbol )
  {
    // case 0x0D: /* Process a carriage return */
    // {
    //   fm = 0; fn = 0;
    //   //========================================
    //   InvalidateRect (hWnd, &myRect, TRUE);
    //   break;
    // }

    case 'q':
    {
      //========================================
      // OnShowDBPoints (wd);
      wd.allPointsDB_show = !wd.allPointsDB_show;
      wd.hStaticBitmapChanged = true;
      //========================================
      InvalidateRect (hWnd, &myRect, FALSE);
      break;
    }

    case 't':
    { 
      //========================================
      wd.trajectory_frames.clear ();
      wd.hand.SET_DEFAULT;

      // // wd.trajectory_frames_muscle = selectHandMove (random (HandMovesCount));
      // wd.trajectory_frames_muscle = wd.hand.selectControl ();
      // wd.trajectory_frames_lasts = random (1U, wd.hand.maxMuscleLast (wd.trajectory_frames_muscle));

      void  fillControlsRandom (IN Hand &hand, OUT HandMoves::controling_t &controls);

      boost::optional<controling_t> controls = controling_t{};
      fillControlsRandom (wd.hand, *controls);

      wd.trajectory_frames.step (wd.store, wd.hand, controls);
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }
    
    case 'o':
    { //========================================
      const size_t  tries = 1000U;
      WorkerThreadRunStoreTask (wd, _T ("\n *** random test ***  "),
                                HandMoves::testRandom,
                                wd.hand, tries);
      WorkerThreadTryJoin (wd);
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }

    case 'p':
    {
      //========================================
      const size_t nested = 2U;
      WorkerThreadRunStoreTask (wd, _T ("\n *** cover test ***  "),
                                HandMoves::testCover,
                                wd.hand, nested);
      WorkerThreadTryJoin (wd);
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }
    
    case '1':
    {
      //========================================
      WorkerThreadRunStoreTask (wd, _T (" *** STAGE 1 ***  "),
                                [](Store &store, Hand &hand, RecTarget &target)
                                {
                                  Positions::LearnMovements lm (store, hand, target);
                                  lm.STAGE_1 ();
                                }
                                , wd.hand, wd.target);
      WorkerThreadTryJoin (wd);
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }

    case '2':
    {
      //========================================
      WorkerThreadRunStoreTask (wd, _T (" *** STAGE 2 ***  "),
                                [](Store &store, Hand &hand, RecTarget &target)
                                {
                                  Positions::LearnMovements lm (store, hand, target);
                                  lm.STAGE_2 ();
                                }
                                , wd.hand, wd.target);
      WorkerThreadTryJoin (wd);
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }

    case '3':
    {
      //========================================
      if ( !wd.testing && !wd.pWorkerThread )
      {
        wd.testing = true;

        tstring  message{ _T (" *** STAGE 3 ***  ") };
        /* Set text of label 'Stat'  */
        SendMessage (wd.hLabTest, WM_SETTEXT, NULL, reinterpret_cast<LPARAM> (message.c_str ()));

        wd.complexity = 0U;
        wd.pWorkerThread = new boost::thread ([](Store &store, Hand &hand, RecTarget &target,
                                                 trajectory_t &uncovered, size_t &complexity)
                                              {
                                                try
                                                {
                                                  Positions::LearnMovements lm (store, hand, target);
                                                  lm.STAGE_3 (uncovered, complexity, false);
                                                }
                                                catch ( boost::thread_interrupted& )
                                                { /* tcout << _T("WorkingThread interrupted!") << std::endl; */ }
                                              }
                                              , std::ref (wd.store),
                                                wd.hand,
                                                std::ref (wd.target),
                                                std::ref (wd.uncoveredPoints),
                                                std::ref (wd.complexity));
      }
      WorkerThreadTryJoin (wd);
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }

    case '0':
    {
      //========================================
      wd.store.clear ();
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }

    case 'j':
    {
      Positions::LearnMovements lm (wd.store, wd.hand, wd.target);
      lm.testStage3 (wd.store, wd.hand, wd.target, wd.uncoveredPoints);
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }

    case 'k':
    {
      //========================================
      WorkerThreadRunStoreTask (wd, _T (" *** Stage 2 test ***  "),
                                [] (Store &store, Hand &hand, RecTarget &target)
                                { 
                                  Positions::LearnMovements lm (store, hand, target);
                                  lm.testStage2 (store, hand, target);
                                }
                                , wd.hand, wd.target);
      WorkerThreadTryJoin (wd);
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }


    case 'u':
    {
      //========================================
      if ( !wd.testing && !wd.pWorkerThread )
      {
        wd.testing = true;

        tstring  message{ _T (" *** Uncover ***  ") };
        /* Set text of label 'Stat'  */
        SendMessage (wd.hLabTest, WM_SETTEXT, NULL, reinterpret_cast<LPARAM> (message.c_str ()));

        wd.pWorkerThread = new boost::thread ([](Store &store, Hand &hand, RecTarget &target,
                                                 trajectory_t &uncovered)
                                              {
                                                try
                                                {
                                                  Positions::LearnMovements lm (store, hand, target);
                                                  lm.uncover (uncovered);
                                                }
                                                catch ( boost::thread_interrupted& )
                                                { /* tcout << _T("WorkingThread interrupted") << std::endl; */ }
                                              }
                                              , std::ref (wd.store),
                                                wd.hand,
                                                std::ref (wd.target),
                                                std::ref (wd.uncoveredPoints));
      }
      WorkerThreadTryJoin (wd);
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }

    case 'i':
    {
      //========================================
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }

    case 'w':
    {
      if ( !wd.working_space.size () )
      {
        Hand::MusclesEnum muscle;
        wd.hand.set (   Hand::JointsSet
                     { {Hand::Clvcl, 0.},
                       {Hand::Shldr, 0.},
                       {Hand::Elbow, 0.},
                       {Hand::Wrist, 0.},
                     });
    
        muscle = Hand::ClvclOpn;
        wd.hand.move (muscle, wd.hand.maxMuscleLast (muscle), wd.working_space);
    
        muscle = Hand::ShldrCls;
        wd.hand.move (muscle, wd.hand.maxMuscleLast (muscle), wd.working_space);
    
        muscle = Hand::ClvclCls;
        wd.hand.move (muscle, wd.hand.maxMuscleLast (muscle), wd.working_space);
    
        muscle = Hand::ElbowCls;
        wd.hand.move (muscle, wd.hand.maxMuscleLast (muscle), wd.working_space);
    
        muscle = Hand::ShldrOpn;
        wd.hand.move (muscle, wd.hand.maxMuscleLast (muscle), wd.working_space);
    
        muscle = Hand::ElbowOpn;
        wd.hand.move (muscle, wd.hand.maxMuscleLast (muscle), wd.working_space);
    
        wd.hand.SET_DEFAULT;
      }
      wd.working_space_show = !wd.working_space_show;
      wd.hStaticBitmapChanged = true;
      InvalidateRect (hWnd, &myRect, FALSE);
      break;
    }

    case 'e':
    {
      if ( wd.pWorkerThread )
      {
        wd.pWorkerThread->interrupt ();
        WorkerThreadTryJoin (wd);
      }
      break;
    }

    case 'g':
    {
      //========================================
      wd.scaleLetters.show = !wd.scaleLetters.show;
      //========================================
      InvalidateRect (hWnd, &myRect, FALSE);
      break;
    } 

    case 'y':
    {
      //========================================
      zoom = !zoom;
      //========================================
      wd.hStaticBitmapChanged = true;
      InvalidateRect (hWnd, &myRect, FALSE);
      break;
    }

    case 'h':
    {
      //========================================
      wd.uncovered_show = !wd.uncovered_show;
      wd.hStaticBitmapChanged = true;
      //========================================
      InvalidateRect (hWnd, &myRect, TRUE);
      break;
    }
    //========================================
    /* Clavicle */
    case 'z': wd.hand.step (Hand::ClvclCls); break; /* двинуть ключицей вправо */
    case 'a': wd.hand.step (Hand::ClvclOpn); break; /* двинуть ключицей влево */
    /* Sholder */
    case 'x': wd.hand.step (Hand::ShldrCls); break;
    case 's': wd.hand.step (Hand::ShldrOpn); break;
    /* Elbow */
    case 'c': wd.hand.step (Hand::ElbowCls); break;
    case 'd': wd.hand.step (Hand::ElbowOpn); break;
    /* Wrist */
    case 'v': wd.hand.step (Hand::WristCls); break;
    case 'f': wd.hand.step (Hand::WristOpn); break;
    /* Reset */
    case 'r': 
    {
      wd.hand.SET_DEFAULT;

      wd.adjPointsDB.clear ();
      wd.trajectoriesDB.clear ();

      wd.mouse_haved = false;

      wd.trajectory_frames.clear ();
      wd.testing_trajectories.clear ();

      wd.uncoveredPoints.clear ();

      /* Setting the Label's text */
      SendMessage (wd.hLabMAim,      /* Label   */
                   WM_SETTEXT,       /* Message */
                   (WPARAM) NULL,    /* Unused  */
                   (LPARAM) _T (" "));
      break;
    }
    //========================================
    // case 'm': if ( !fn ) fm = 1; break;
    // case 'n': if ( !fm ) fn = 1; break;

    // case '0': case '1': case '2': case '3': case '4':
    // case '5': case '6': case '7': case '8': case '9':
    // {
    //   //      if ( fm ) p_x = p_x * 10U + ((uchar_t) wParam - 0x30);
    //   // else if ( fn ) p_y = p_y * 10U + ((uchar_t) wParam - 0x30);
    // 
    //   // if ( p_x >= tgRowsCount ) p_x = tgRowsCount - 1U;
    //   // if ( p_y >= tgColsCount ) p_y = tgColsCount - 1U;
    //   //========================================
    //   break;
    // }
  }
}
//-------------------------------------------------------------------------------
tstring   OpenFileDialog (HWND hWnd)
{
  /* common dialog box structure */
  OPENFILENAME  OpenFileName = {};
  TCHAR         szFilePath[MAX_PATH];  /* buffer for file name */
  TCHAR         szFileName[MAX_PATH];

  /* Initialize OpenFileName */
  OpenFileName.lStructSize = sizeof (OPENFILENAME);
  OpenFileName.hwndOwner = hWnd;

  OpenFileName.lpstrFileTitle = szFileName;
  OpenFileName.nMaxFileTitle = sizeof (szFileName);
  OpenFileName.lpstrFile = szFilePath;
  OpenFileName.nMaxFile = sizeof (szFilePath);

  /*  GetOpenFileName does not use the
  *  contents to initialize itself. 
  */
  OpenFileName.lpstrFile[0] = '\0';
  OpenFileName.lpstrFileTitle[0] = '\0';

  OpenFileName.lpstrDefExt = _T ("bin");
  OpenFileName.lpstrFilter = _T ("Binary\0*.bin\0All\0*.*\0");
  OpenFileName.nFilterIndex = 1;
  OpenFileName.lpstrInitialDir = NULL;
  OpenFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

  // Display the Open dialog box. 
  if ( !GetOpenFileName (&OpenFileName) )
  { return  _T (""); }
  return  OpenFileName.lpstrFile;
}
tstring   SaveFileDialog (HWND hWnd)
{
  OPENFILENAME  SaveFileName = {};
  TCHAR         szFileName[MAX_PATH] = {};
  TCHAR         szFilePath[MAX_PATH] = {};

  SaveFileName.lStructSize = sizeof (OPENFILENAME);
  SaveFileName.hwndOwner = hWnd;
  
  SaveFileName.lpstrFile = szFilePath;
  SaveFileName.nMaxFile = sizeof (szFilePath);
  SaveFileName.lpstrFileTitle = szFileName;
  SaveFileName.nMaxFileTitle = sizeof (szFileName);

  SaveFileName.lpstrFile[0] = '\0';
  SaveFileName.lpstrFileTitle[0] = '\0';

  SaveFileName.lpstrDefExt = _T ("bin");
  SaveFileName.lpstrFilter = _T ("Binary\0*.bin\0All\0*.*\0");
  SaveFileName.nFilterIndex = 1;
  SaveFileName.lpstrInitialDir = NULL;

  SaveFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

  // Display the Open dialog box. 
  if ( GetSaveFileName (&SaveFileName) )
  { return _T(""); }
  return SaveFileName.lpstrFile;
}
//-------------------------------------------------------------------------------
tstring   GetTextToString (HWND hWnd)
{
  int  len = GetWindowTextLength (hWnd) + 1U;
  std::vector<TCHAR>  buffer (len);
  GetWindowText (hWnd, buffer.data (), len);
  // SendMessage (hWnd, WM_GETTEXT, (WPARAM) len, (LPARAM) buffer.data ());
  return  tstring (buffer.begin (), buffer.end () - 1);
}
//-------------------------------------------------------------------------------
tstring   CurrentTimeToString (tstring format, std::time_t *the_time)
{
  std::time_t rawtime;
  if ( !the_time )
  { rawtime = std::time (nullptr); }
  else
  { rawtime = *the_time; }
  struct tm  *TimeInfo = std::localtime (&rawtime);

  tstringstream ss;
  ss << std::put_time (TimeInfo, format.c_str ());
  return ss.str ();
}
//-------------------------------------------------------------------------------
void      MakeGradient (color_interval_t  colors, size_t n_levels,
                        gradient_t  &gradient)
{ 
  gradient.clear ();
  gradient.resize (n_levels);

  COLORREF cf = colors.first, cs = colors.second;
  /* loop to create the gradient */
  for ( size_t i = 0U; i < n_levels; ++i )
  {
    unsigned char  r, g, b;
    /* Determine the colors */
    r = static_cast<unsigned char> (GetRValue (cf) + (i * (GetRValue (cs) - GetRValue (cf)) / n_levels));
    g = static_cast<unsigned char> (GetGValue (cf) + (i * (GetGValue (cs) - GetGValue (cf)) / n_levels));
    b = static_cast<unsigned char> (GetBValue (cf) + (i * (GetBValue (cs) - GetBValue (cf)) / n_levels));
    /* Append new color */
    gradient[i] = RGB (r, g, b);
  }
}
//-------------------------------------------------------------------------------
void      OnEraseBackGround_WithGradient (HWND hwnd)
{
  /* Vars */
  HDC dc; /* Standard Device Context; used to do the painting */

          /* rect = Client Rect of the window;
          Temp = Temparary rect tangle for the color bands */
  RECT rect, temp;
  HBRUSH color; /* A brush to do the painting with */

                /* Get the dc for the wnd */
  dc = GetDC (hwnd);

  /* Get the client rect */
  GetClientRect (hwnd, &rect);

  /* Start color; Change the R,G,B values
  to the color of your choice */
  int r1 = 255, g1 = 0, b1 = 0;

  /* End Color; Change the R,G,B values
  to the color of your choice */
  int r2 = 255, g2 = 255, b2 = 0;

  /* loop to create the gradient */
  for ( int i = 0; i<rect.right; i++ )
  {
    /* Color ref. for the gradient */
    int r, g, b;
    /* Determine the colors */
    r = r1 + (i * (r2 - r1) / rect.right);
    g = g1 + (i * (g2 - g1) / rect.right);
    b = b1 + (i * (b2 - b1) / rect.right);

    /* Fill in the rectangle information */

    /* The uper left point of the rectangle
    being painted; uses i as the starting point*/
    temp.left = i;
    /* Upeer Y cord. Always start at the top */
    temp.top = 0;
    /* Okay heres the key part,
    create a rectangle thats 1 pixel wide */
    temp.right = i + 1;
    /* Height of the rectangle */
    temp.bottom = rect.bottom;

    /* Create a brush to draw with;
    these colors are randomized */
    color = CreateSolidBrush (RGB (r, g, b));

    /* Finally fill in the rectange */
    FillRect (dc, &temp, color);
  }
}
//-------------------------------------------------------------------------------
