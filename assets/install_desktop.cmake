find_program(XDG_ICON_EXECUTABLE xdg-desktop-icon)
find_program(XDG_DESKTOP_EXECUTABLE xdg-desktop-menu)
execute_process(COMMAND ${XDG_ICON_EXECUTABLE} install ${CMAKE_CURRENT_BINARY_DIR}/assets/modtool.png)
execute_process(COMMAND ${XDG_DESKTOP_EXECUTABLE} install --novendor ${CMAKE_CURRENT_BINARY_DIR}/assets/modtool.desktop)
