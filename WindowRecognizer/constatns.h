#pragma once

namespace constants
{
  // Main Window params
  const int wnd_height{ 300 };
  const int wnd_width{ 800 };

  // Edit Controls params
  const int ec_width{ 200 };
  const int ec_height{ 25 };
  const int ec_bottom_offset{ 5 };
  const int ec_left_offset{ 150 };
  const int ec_top_offset{ 30 };

  // Buttons params
  const wchar_t btn_find_window[]{ L"Найти окно" };
  const wchar_t btn_save[]{ L"Сохранить" };
  const int btn_width{ 150 };
  const int btn_height{ 25 };
  const int btn_x_offset{ 150 };
  const int btn_y_offset{ 10 };

  // Titles for Edit Controls
  const wchar_t ec_class[]{ L"Класс окна" };
  const wchar_t ec_title[]{ L"Заголовок окна" };
  const wchar_t ec_app_path[]{ L"Приложение" };

  // File name to save data
  const wchar_t file_name[]{ L"window.txt" };

  //Font name
  const wchar_t font_name[]{ L"Comic Sans" };
}
