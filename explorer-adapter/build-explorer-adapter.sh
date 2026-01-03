#!/bin/bash
# Run this inside the folder containing explorer-adapter.c to build
# Dependencies: winegcc, mingw-w64-gcc
winegcc -target x86_64-w64-mingw32 explorer-adapter.c -o explorer.exe
