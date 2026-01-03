@echo off

:: Legacy script used initially to make the explorer.exe adapter.
:: Due to many bat to exe conversion tools being flagged by antivirus software
:: I build a C-based version instead. This is still here in case anyone finds it useful

setlocal enabledelayedexpansion
if "%~1"=="" goto :execute_fallback


:parse_options_with_path
set "is_select=0"
if /i "%~1"=="/root" (
    if "%~2"=="" goto :parse_path_only
    set "target_path=%~2"
    goto :cleanup_and_validate_path
)
if /i "%~1"=="/select" (
    if "%~2"=="" goto :parse_path_only
    set "is_select=1"
    set "target_path=%~2"
    goto :cleanup_and_validate_path
)
:parse_path_only
set "target_path=%~1"


:cleanup_and_validate_path
:: Remove quotes
set "target_path=!target_path:"=!"
:: Strip file:/// prefix if present
if /i "!target_path:~0,8!"=="file:///" (
    set "target_path=!target_path:~8!"
)
:: Check if arg1 starts with drive letter + :
set "drive_letter=!target_path:~0,1!"
set "drive_colon=!target_path:~1,1!"
if /i "!drive_letter!" geq "A" if /i "!drive_letter!" leq "Z" if "!drive_colon!"==":" (
    goto :process_path
)
goto :execute_fallback


:process_path
:: Convert Windows path to Unix path
set "unix_path="
for /f "delims=" %%a in ('winepath.exe --unix "!target_path!"') do set "unix_path=%%a"
if defined unix_path (
    set "redirect_handler=~/.wine"
    if not "%WINEPREFIX%"=="" (
        set "redirect_handler=!WINEPREFIX!"
    )
    set "redirect_handler=%WINEPREFIX%"
    if "!redirect_handler:~-1!"=="/" set "redirect_handler=!redirect_handler:~0,-1!"
    set "redirect_handler=!redirect_handler!/explorer-redirect/redirect-path"
    
    :: If /select then select, other wise just open folder
    :: Using very very funky syntax with all kinds of quotes here to allow spaces for both, redirect_handler and unix_path
    if "%is_select%"=="1" (
        :: Select file or folder (and probably open parent folder)
        echo Selecting "!unix_path!" converted from "!target_path!"
        start /unix /bin/bash -c "\"!redirect_handler!\" --select '\"!unix_path!\"'"
        exit /b
    ) else (
        :: Open folder (or parent folder if file)
        echo Opening "!unix_path!" converted from "!target_path!"
        start /unix /bin/bash -c "\"!redirect_handler!\" '\"!unix_path!\"'"
        exit /b
    )
)


:: Fall back to the original explorer.exe for all other cases
:execute_fallback
start "" C:\Windows\explorer.exe %*

