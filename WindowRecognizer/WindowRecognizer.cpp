#include "constatns.h"
#include "framework.h"
#include "WindowRecognizer.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
// App title string.
WCHAR szTitle[MAX_LOADSTRING];
// Main window class name.
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// To access correct cyrillic symbols output to file, have to add locale.
const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());

HWND hwnd_already_found = NULL;
HWND hwnd_own = NULL;

bool is_already_looking_for = false;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Global strings init.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWRECOGNIZER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWRECOGNIZER));

    MSG msg;

    // Main message loop
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


// Register window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWRECOGNIZER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWRECOGNIZER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// Create and show main app window.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;
   // Exclude everythinng regarding resize for window to save sizes of window.
   // Due too app purposes we don't need it to be full screened or minimized.
   HWND hWnd = CreateWindowExW(WS_EX_TOPMOST, szWindowClass, szTitle,
                               WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX &
                               ~WS_MINIMIZEBOX & ~WS_THICKFRAME,
                               CW_USEDEFAULT, 0, 
                               constants::wnd_width, constants::wnd_height,
                               nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   hwnd_own = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void ClaimAndShowInfoOnFoundWindow(HWND hWnd, HWND hWnd_found)
{
  TCHAR wnd_class_name[MAX_PATH] = {0};
  TCHAR wnd_title[MAX_PATH] = {0};
  TCHAR wnd_app_path[MAX_PATH] = {0};
  DWORD dwProcId = 0;

  // Get the class name of found window.
  GetClassName(hWnd_found, wnd_class_name, _countof(wnd_class_name));
  SetDlgItemText(hWnd, IDM_EDIT_WINDOW_CLASS, wnd_class_name);

  // Get the the title of found window.
  GetWindowText(hWnd_found, wnd_title, _countof(wnd_title));
  SetDlgItemText(hWnd, IDM_EDIT_WINDOW_TITLE, wnd_title);

  // Access to process of found window and retrieve app path for it.
  GetWindowThreadProcessId(hWnd_found, &dwProcId);
  HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcId);
  DWORD n = MAX_PATH;
  QueryFullProcessImageName(hProc, 0, wnd_app_path, &n);
  CloseHandle(hProc);
  SetDlgItemText(hWnd, IDM_EDIT_WINDOW_APP_PATH, wnd_app_path);
}

bool IsValidWindow(HWND hWnd, HWND hWnd_found)
{
  // Check that window is not NULL
  if (hWnd_found == NULL)
    return false;

  // It must also be a valid window as far as the OS is concerned.
  if (IsWindow(hWnd_found) == FALSE)
    return false;

  // Ensure that the window is not the current one which has already been found.
  if (hWnd_found == hwnd_already_found)
    return false;

  // It must also not be the main window itself.
  if (hWnd_found == hWnd)
    return false;

  return true;
}

void MouseEventAnalys(const POINT &pt)
{
  POINT	scr_point;
  HWND hwnd_found = NULL;

  hwnd_found = WindowFromPoint(pt);

  // Get the top most parent for found window ot exclude buttons from search.
  hwnd_found = GetAncestor(hwnd_found, GA_ROOT);

  if (IsValidWindow(hwnd_own, hwnd_found))
  {
    // Claim and show information about found window.
    ClaimAndShowInfoOnFoundWindow(hwnd_own, hwnd_found);

    // Remember window to not process it on next event handler.
    hwnd_already_found = hwnd_found;
  }
}

LRESULT WINAPI MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
  MSLLHOOKSTRUCT* hook_struct = (MSLLHOOKSTRUCT*)lParam;
  if (nCode == HC_ACTION && hook_struct != NULL) {
    MouseEventAnalys(hook_struct->pt);
  }

  return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
      RECT rc_window;
      GetClientRect(hWnd, &rc_window);

      // Buttons creation
      auto AddButton = [hWnd](int dialog_item_id, int x, int y, const wchar_t *wnd_text) {
        CreateWindow(L"button", wnd_text,
                     WS_VISIBLE | WS_CHILD,
                     x, y, constants::btn_width, constants::btn_height,
                     hWnd, (HMENU)dialog_item_id, NULL, NULL);
      };

      AddButton(IDM_BUTTON_FIND_WINDOW, rc_window.left + constants::btn_x_offset,
          rc_window.bottom - constants::btn_height - constants::btn_y_offset, constants::btn_find_window);
      AddButton(IDM_BUTTON_SAVE, rc_window.right - constants::btn_x_offset - constants::btn_width,
          rc_window.bottom - constants::btn_height - constants::btn_y_offset, constants::btn_save);


      // Text fields creation
      int third_of_window_width = (rc_window.right - rc_window.left) / 3;
      int current_top_pos = rc_window.top + constants::ec_top_offset;

      auto AddEditBox = [hWnd, third_of_window_width](int dialog_item_id, int top_pos) {
        HWND edit_hwnd = CreateWindow(L"edit", L"", WS_BORDER | WS_VISIBLE | WS_CHILD |
                                      ES_READONLY | ES_CENTER | ES_AUTOHSCROLL,
                                      third_of_window_width, top_pos,
                                      third_of_window_width, constants::ec_height,
                                      hWnd, (HMENU)dialog_item_id, NULL, NULL);

        HFONT edit_font = CreateFont(-14, 0, 0, 0, 0, 0u, 0U, 0U,
                                     ANSI_CHARSET, 0U, 0U, 0U, 0U, constants::font_name);
        SendMessage(edit_hwnd, WM_SETFONT, (WPARAM)edit_font, TRUE);
      };

      AddEditBox(IDM_EDIT_WINDOW_CLASS, current_top_pos);
      current_top_pos += constants::ec_height + constants::ec_bottom_offset;
      AddEditBox(IDM_EDIT_WINDOW_TITLE, current_top_pos);
      current_top_pos += constants::ec_height + constants::ec_bottom_offset;
      AddEditBox(IDM_EDIT_WINDOW_APP_PATH, current_top_pos);

      break;
    }
    case WM_COMMAND:
    {
      int wmId = LOWORD(wParam);

      switch (wmId)
      {
      case IDM_BUTTON_FIND_WINDOW:
        if (!is_already_looking_for) {
          SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, NULL, 0);
          is_already_looking_for = true;
        }
        break;
      case IDM_BUTTON_SAVE: 
      {
        std::wofstream output_file(constants::file_name, std::ios::app);
        output_file.imbue(utf8_locale);
        if (output_file.is_open())
        {
          std::wstring file_line(L"Path - ");
          // Taking edit control content and put into resulting string
          auto ReplacePlaceholders = [hWnd](int dialog_item_id, std::wstring& 
              file_line, const std::wstring& add_string) {
            TCHAR buffer[MAX_PATH] = { 0 };

            GetDlgItemText(hWnd, dialog_item_id, buffer, MAX_PATH);
            file_line += buffer;
            file_line += add_string;
          };
          
          ReplacePlaceholders(IDM_EDIT_WINDOW_APP_PATH, file_line, L"; Class - ");
          ReplacePlaceholders(IDM_EDIT_WINDOW_CLASS, file_line, L"; Header - ");
          ReplacePlaceholders(IDM_EDIT_WINDOW_TITLE, file_line, L";");
          output_file << file_line << std::endl;
          output_file.close();
        }
        break;
      }
      default:
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
      break;
    }
    case WM_ERASEBKGND: 
    {
      HDC hdc = (HDC)(wParam);
      RECT rc; 
      GetClientRect(hWnd, &rc);
      HBRUSH brush = CreateSolidBrush(RGB(220, 220, 220));
      FillRect(hdc, &rc, brush);
      DeleteObject(brush);
      
      break;
    }
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);

      RECT rc_window, rct_class_title, rct_header_title, rct_app_path_title;
      GetClientRect(hWnd, &rc_window);
      int third_of_window_width = (rc_window.right - rc_window.left) / 3;
      int current_top_pos = rc_window.top + constants::ec_top_offset;

      rct_class_title = rc_window;
      rct_class_title.top += constants::ec_top_offset;
      rct_class_title.right = third_of_window_width;
      rct_class_title.bottom = rct_class_title.top + constants::ec_height;

      rct_header_title = rct_class_title;
      rct_header_title.top += constants::ec_height + constants::ec_bottom_offset;
      rct_header_title.bottom = rct_header_title.top + constants::ec_height;

      rct_app_path_title = rct_header_title;
      rct_app_path_title.top += constants::ec_height + constants::ec_bottom_offset;
      rct_app_path_title.bottom = rct_app_path_title.top + constants::ec_height;

      InflateRect(&rct_class_title, -5, 0);
      InflateRect(&rct_header_title, -5, 0);
      InflateRect(&rct_app_path_title, -5, 0);

      SetBkMode(hdc, TRANSPARENT);
      SetBkColor(hdc, RGB(255, 255, 255));
      HFONT new_font, old_font;
      new_font = CreateFont(-20, 0, 0, 0, 0, 0u, 0U, 0U, 
                            ANSI_CHARSET, 0U, 0U, 0U, 0U, constants::font_name);

      if (old_font = (HFONT)SelectObject(hdc, new_font))
      {
        DrawText(hdc, constants::ec_class, _countof(constants::ec_class),
          &rct_class_title, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
        DrawText(hdc, constants::ec_title, _countof(constants::ec_title),
          &rct_header_title, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
        DrawText(hdc, constants::ec_app_path, _countof(constants::ec_app_path),
          &rct_app_path_title, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

        // Restore the original font.        
        SelectObject(hdc, old_font);

      }
      DeleteObject(new_font);
      EndPaint(hWnd, &ps);

      break;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
