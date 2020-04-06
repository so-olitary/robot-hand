﻿#include "StdAfx.h"
#include "WindowHeader.h"
#include "WindowData.h"


#ifdef MY_WINDOW
int WINAPI  WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  HWND      hWnd;
  MSG       lpMsg;
  WNDCLASS  wc;
  TCHAR     szClassName[] = _T("Graph interface window");

  // Заполняем структуру класса окна
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = WndProc;
  wc.cbClsExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = NULL;      
  wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = szClassName;
  wc.cbWndExtra    = sizeof(MyWindowData*); // выделяем место под указатель на пользовательские данные в hWnd
  
  // Регистрируем класс окна
  if( !RegisterClass (&wc) )
  { MessageBox (NULL, _T("Can't register window class."), _T("Error"), MB_OK);
    return 0;
  }
  //=======================
  // Создаём дополнительное окно с коммандной строкой
  // Перенаправляем в неё стандартные потоки ввода/вывода
  redirectConsoleIO();
  // Изменяем заголовок консоли
  SetConsoleTitle(_T("cmd-robo-moves"));
  //=======================
  Utils::CArgs args;
  // Получаем параметры командной строки
  getConsoleArguments(args);
  //=======================
  init_thread_rand_seed();
  //=======================
  // Инициализируем пользовательские данные
  MyWindowData wd(args);
  //=======================
  // Создаем основное окно приложения
  hWnd = CreateWindow ( szClassName,                // Имя класса                    
                        _T("robot-moves"),          // Текст заголовка 
                        WS_OVERLAPPEDWINDOW,        // Стиль окна                                              
                        0, 0,                       // Позиция левого верхнего угла   
                        myWIDTH, myHIGHT,           // Ширина и высота окна     
                        (HWND) NULL,                // Указатель на родительское окно NULL     
                        (HMENU) NULL,               // Используется меню класса окна               
                        (HINSTANCE) hInstance,      // Указатель на текущее приложение
                        (LPVOID) &wd );             // Передается в качестве lParam в событие WM_CREATE
  
  if( !hWnd ) 
  { MessageBox (NULL, _T("Can't create main window!"), _T("Error"), MB_OK); 
    return 0;
  }
  
  // Показываем наше окно
  ShowWindow   (hWnd, nCmdShow); 
  UpdateWindow (hWnd);
  
  // Выполняем цикл обработки сообщений до закрытия приложения
  while ( GetMessage (&lpMsg, NULL, 0, 0) )  
  { TranslateMessage (&lpMsg);
     DispatchMessage (&lpMsg);
  }
  return int(lpMsg.wParam);
}

#else //!MY_WINDOW

#include "Test/Test.h"

int main(const int argc, const TCHAR **argv)
{
    Utils::CArgs args;
    getConsoleArguments(argc, argv, args);
    //=======================
    init_thread_rand_seed();
    //=======================
    try
    {
        MyWindowData wd(args);
        test::Test tests(wd.pCArgs->testsfile, wd.pCArgs->testname);
    }
    catch (std::exception &e)
    {
        SHOW_CERROR(e.what());
        return 1;
    }
    return 0;
}

#endif //!MY_WINDOW
