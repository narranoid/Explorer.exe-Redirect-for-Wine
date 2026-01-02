# Explorer.exe Redirect for Wine

**Tired of your Wine apps opening the winefile explorer instead of your system's default file manager?**

Redirect explorer.exe calls for opening and selecting folders/files to a native file manager inside a Wine prefix with these scripts.

![Sync.com client opening nautilus instead of explorer.exe / winefile.exe](explorer-redirect-demo.jpg)

Tested with the [NonCloudFiles version of the Sync.com client](https://appdb.winehq.org/objectManager.php?sClass=application&iId=17380) and **should support other programs**. 
After following the install instructions, your Sync.com client (or other software if supported) will open folders and select files in your native file manager.

The current version can only be used with programs that are able to run an explorer.exe from their program folder alternatively to the C:\windows\explorer.exe, which is the fallback. 
However, a few script modifications may already be enough to run it globally in a Wine prefix. 

## How it Works
The explorer-adapter.bat is converted into an explorer.exe and if called from another program to simply open a folder or to select a file, it will redirect that call to the native file manager.

Specifically, the redirect invokes explorer-redirect/redirect-path which is a bash script receiving the unix filepaths (translated via winepath.exe) and the optional select instruction as parameters.

## Install Instructions
Installation is per wine prefix.

1. Download and extract the latest release (if you don't have it already). 
2. Copy the explorer-redirect folder into the root folder of your wine prefix (as in the prefix root and not the drive_c folder).
3. Put the explorer.exe either in:
    - The program folder of a certain program that should use this explorer.exe instead (not every program may support this).
    - Make a backup copy of `C:\windows\explorer.exe` and paste it under a different name in the same folder. Then replace `C:\windows\explorer.exe` with the explorer.exe converted from explorer-adapter.bat.
4. You may need to add a dot character to `HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment\PATHEXT` in the registry of your prefix as explained [here](https://gitlab.winehq.org/wine/wine/-/wikis/FAQ#how-do-i-launch-native-applications-from-a-windows-application), so Wine can launch native applications.
5. Now, whenever Wine attempts to open a folder or select a file, your native file manager should open instead!

**Important Note:**

Replacing C:\windows\explorer.exe will require a modification of the explorer-adapter.bat and a re-conversion of it (see [Build Instructions](#Build-Instructions)). You cannot use the explorer.exe from the latest release for it and you need to figure out instead how to properly fallback to the backup version of the prefix' original explorer.exe so it actually runs properly!

(I will attempt to provide a more out-of-the-box solution for this install method in the future.)


## Troubleshooting

**If your system struggles to open the internal file manager:**

Try to modify the explorer-redirect/redirect-path script so it simply calls your file manager. The script receives an optional --select as parameter and the file path as the other parameter.
With some knowledge of bash scripts it should be straight forward to adjust it to your needs. 

**If Wine struggles to use the custom explorer.exe converted from explorer-adapter.bat:**

Try to modify the explorer-adapter.at beneath the execute_fallback area and tinker how to invoke the actual explorer.exe in a different way if needed.
Then convert the bat script again to an exe as explained in the Build Instructions.


## Build Instructions

To make a modified version of the explorer-adapter.bat and convert it to an exe file:

1. Clone this repository or get the source code from the latest release (if you don't have it already).
2. Modify the explorer-adapter.bat so it fits your needs.
3. Convert it to an exe file.
    - I used [BatToExePortable](https://github.com/Makazzz/BatToExePortable) for this which runs with Wine but other converters should be fine too.
    - Note that this is required as Wine expects a proper binary exe when calling explorer.exe.
4. Follow the [Install Instructions](#Install-Instructions) using your new explorer.exe.


