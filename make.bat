@echo off

SET GBDK_HOME=C:/gbdk

mkdir dist

SET PNG2ASSET=%GBDK_HOME%/bin/png2asset.exe

:: Sprites
%PNG2ASSET% graphics/mario.png -c graphics/mario.c  -sw 16 -sh 32 -spr8x16 -keep_palette_order
%PNG2ASSET% graphics/mushroom.png -c graphics/mushroom.c  -sw 16 -sh 16 -spr8x16 -keep_palette_order
%PNG2ASSET% graphics/font.png -c graphics/font.c  -keep_palette_order -map

:: Compile a .gb file from the compiled .o files
%GBDK_HOME%\bin\lcc -Wm-yC -Wl-yt3 -o dist/SubPixelResolution.gb main.c graphics/mario.c graphics/mushroom.c graphics/font.c
