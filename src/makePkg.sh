#!/bin/bash
# PowerBudget v0.00.01a 2025/08/16 calculate power dissipation and budget
# makePkg.sh: Copyright 2005-2025 Valerio Messina efa@iol.it
# makePkg is part of PowerBudget
# PowerBudget is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# PowerBudget is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with PowerBudget. If not, see <http://www.gnu.org/licenses/>.
#
# Script to generate a Linux|Mingw|MXE|OSX package of 'PowerBudget'
# used on: Linux=>bin64, Linux=>bin32, Linux=>macOS64
#          MinGw64=>bin64, MinGw32=>bin32, MXE64=>bin64, MXE32=>bin32
#
# Syntax: $ makePkg.sh [-y] Linux|WinMxe|WinMgw|MacOS [32|64]

makever=2025-08-16

DEPSPATHMGW64="/mingw64/bin" # path of DLLs needed to generate the Mingw64 package
DEPSPATHMGW32="/mingw32/bin" # path of DLLs needed to generate the Mingw32 package
DEPSLISTMGW="" # list of dll for MinGW

APP="PowerBudget"     # app name
SRCPKG=".." # source package path
BIN="powerBudgetGui" # binary
DSTPATH=".." # path where create the Linux|Mingw/MXE|OSX package directory

echo "makePkg.sh: create a Linux|WinMxe|WinMgw|MacOS package for $APP ..."

# check for external dependency compliance
flag=0
for extCmd in 7z chmod cp cut date genisoimage grep mkdir mv pwd rm tar uname wget ; do
   exist=`which $extCmd 2> /dev/null`
   if (test "" = "$exist") then
      echo "Required external dependency: "\"$extCmd\"" unsatisfied!"
      flag=1
   fi
done
if [[ "$flag" = 1 ]]; then
   echo "ERROR: Install the required packages and retry. Exit"
   exit
fi

if (test "$1" = "-y") then
   batch=1
   shift
fi
if [[ "$1" = "" || "$1" != "Linux" && "$1" != "WinMxe" && "$1" != "WinMgw" && "$1" != "MacOS" ]]; then
   echo "ERROR: makePkg.sh unsupported/miss target platform to create package"
   echo "Syntax: $ makePkg.sh [-y] Linux|WinMxe|WinMgw|MacOS [32|64]"
   echo "          -y for batch execution without confirmations"
   exit
fi

#exist=`which gtk-mac-bundler 2> /dev/null`
#if (test "$1" = "MacOS" && test "" = "$exist") then
#   echo "ERROR: makePkg.sh depend on 'gtk-mac-bundler' to generate for macOS. Exit"
#   exit
#fi
exist=`which osxcross-dmg 2> /dev/null`
if (test "$1" = "OSX64" && test "" = "$exist") then
   echo "ERROR: makePkg.sh depend on 'osxcross-dmg' to generate for macOS. Exit"
   exit
fi

PKG="$1"
CPU=`uname -m` # i686 or x86_64
if (test "" = "$2") then
   BIT=$(getconf LONG_BIT)
else
   BIT="$2"
fi

BIN="$BIN$PKG$BIN"
if [[ -f $BIN ]]; then
   mkdir -p $DSTPATH
else
   echo "ERROR: makePkg.sh cannot find just built binary './$BIN'. Exit"
   exit
fi

if (test "$CPU" = "x86_64" && test "$BIT" = "32") then
   CPU=i686
fi
if (test "$PKG" = "WinMxe" || test "$PKG" = "WinMgw") then
   EXT=".exe"
fi

OS=`uname`
if (test "$OS" != "Darwin") then
   OS=`uname -o`  # Msys or GNU/Linux, illegal on macOS
fi
VER=`grep SourceVersion powerbLib.h | cut -d' ' -f3 | tr -d '."'`
DATE=`date -I`
BINPATH=`pwd`
TGT=$PKG
if [[ "$OS" = "Msys" ]]; then
   if [[ "$BIT" = "64" ]]; then
      DEPSRC=$DEPSPATHMGW64
   fi
   if [[ "$BIT" = "32" ]]; then
      DEPSRC=$DEPSPATHMGW32
   fi
fi
UPKGPAT="" # Uncompressed Package Path
if [[ "$PKG" = "MacOS" ]]; then
   TGT=Osx
   UPKGPAT="DiskImage/$APP.app/Contents/MacOS"
   DSTSP="$UPKGPAT"
fi
if [[ "$PKG" = "Linux" ]]; then
   TGT=Linux
   UPKGPAT="usr/bin"
   DSTSP="usr/src"
fi
DSTPKG="${APP}_${VER}_${DATE}_${TGT}_${CPU}_${BIT}bit"

if [[ "$OS" != "Msys" && "$OS" != "GNU/Linux" ]]; then
   echo "ERROR makePkg.sh: work in Linux|WinMxe(Linux)|WinMgw only"
   exit
fi
if [[ "$OS" = "Msys" && "$PKG" != "WinMgw" ]]; then
   echo "ERROR makePkg.sh: Unsupported target package:$PKG on MinGW/MSYS2"
   exit
fi

if [[ "$OS" = "GNU/Linux" && "$PKG" != "Linux" && "$PKG" != "WinMxe" && "$PKG" != "MacOS" ]]; then
   echo "ERROR makePkg.sh: Unsupported target package:$PKG on Linux"
   exit
fi

echo "DATE   : $DATE"    # today
echo "APP    : $APP"     # app name
echo "VER    : $VER"     # src ver
echo "PKG    : $PKG"     # input par
echo "TGT    : $TGT"     # req target OS
echo "TGTBIT : $BIT"     # req target BIT
echo "TGTCPU : $CPU"     # target CPU
echo "HOSTOS : $OS"      # current OS
echo "SRCPKG : $SRCPKG"  # packaging file source path
echo "BINPATH: $BINPATH" # binary path
echo "BIN    : $BIN"     # binary file
if (test "$DEPSRC" != "") then
echo "SRCDEP : $DEPSRC"  # dependancy path (MinGw)
fi
echo "DSTPATH: $DSTPATH" # package creation path
echo "DSTSP  : $DSTSP"   # app sources destination path
echo "UPKGPAT: $UPKGPAT" # package uncompressed creation path
echo "DSTPKG : $DSTPKG"  # package name
if (test "$batch" != "1") then
   read -p "Proceed? A key to continue"
fi
echo ""

echo "makePkg: Creating $APP $VER package for $TGT$BIT.$CPU ..."
cp -a ../Readme.txt ../README.md

if (test "$PKG" = "MacOS") then
   if (test "$BIT" = "32") then
      echo "Unsupported 32 bit on MacOS"
      exit
   fi
   #gtk-mac-bundler $APP.bundle
   cp -a ${APP}MacOS $APP.icns AppDir
   cd AppDir
   osxcross-dmg -rw ${APP}MacOS $APP $VER
   rm uncompressed.dmg ${APP}MacOS $APP.icns
   mv $APP$VER.dmg ..
   cd ..
   #mkdir $APP.app
   #cd ../..
   #AppName=powerBudgetGuiMacOS64
   #ls -l $AppName.app
   #tar -cf $AppName.app.tgz $AppName.app
   #wp=0 # write protect
   #echo "Generating uncompressesd DMG ..."
   #rm -rf DiskImage 2>/dev/null
   #mkdir DiskImage
   #mv $AppName.app DiskImage
   #rm $AppName$VER.dmg 2> /dev/null
   #if (test "$wp" = 1) then
   #   genisoimage -V $AppName -D -r -apple -no-pad -o $AppName$VER.dmg DiskImage
   #else
   #   genisoimage -V $AppName -D -R -apple -no-pad -o $AppName$VER.dmg DiskImage
   #fi
   #rm -rf DiskImage
   #echo "Compressing DMG ..."
   #mv $AppName$VER.dmg uncompressed.dmg
   # dmg from: https://github.com/fanquake/libdmg-hfsplus
   #dmg uncompressed.dmg $AppName$VER.dmg
   #rm uncompressed.dmg
   #echo "$AppName$VER.dmg created."
   exit
fi

if [[ -d "$DSTPATH/AppDir" ]]; then
   echo "Removing old AppDir ..."
   rm -rf "$DSTPATH/AppDir"
fi
mkdir -p "$DSTPATH/AppDir/$UPKGPAT"

if [[ -f "$DSTPATH/AppImage/$DSTPKG.AppImage" ]]; then
   echo "Removing old AppImage ..."
   rm -f "$DSTPATH/AppImage/$DSTPKG.AppImage"
fi
if [[ -d "$DSTPATH/AppImage/AppDir" ]]; then
   echo "Removing old AppImageDir ..."
   rm -rf "$DSTPATH/AppImage/AppDir"
fi

if [[ -d $DSTPATH/$DSTPKG ]]; then
   echo "Removing old DSTPKG ..."
   rm -rf "$DSTPATH/$DSTPKG"
fi
mkdir -p $APP/src

cd ..
cp -a Readme.txt LICENSE src/AppDir/usr/bin
cp -a Readme.txt LICENSE src/PowerBudget
cp -a powerBudget${PKG}${BIT}${EXT}    src/AppDir/usr/bin/
cp -a powerBudget${PKG}${BIT}${EXT}    src/PowerBudget/
cp -a powerBudgetGui${PKG}${BIT}${EXT} src/AppDir/usr/bin/
cp -a powerBudgetGui${PKG}${BIT}${EXT} src/PowerBudget/
cd src
cp -a powerb.ini AppDir/usr/bin/src
cp -a powerb.ini PowerBudget/src
cp -a makePkg.sh PowerBudget/src
cp -a *.h *.c    PowerBudget/src
cp -a Makefile*  PowerBudget/src
cp -a PowerBudget.png powerBudget.desktop AppDir
cp -a PowerBudget.png powerBudget.desktop PowerBudget/src
cp -a PowerBudget.ico PowerBudget.icns Info.plist PowerBudget/src 2>/dev/null

if (test "$PKG" = "Linux" && (test "$CPU" = "x86_64" || test "$CPU" = "i686")) then # skip on ARM&RISC-V
   echo "makePkg.sh: generating the AppImage for PowerBudget (about 20\") ..."
   if (test -f logWget$DATE.txt) then { rm logWget$DATE.txt ; } fi
   if (test "$BIT" = "64") then
      if (! test -x linuxdeploy-x86_64.AppImage) then
         wget -nv "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" 2>> logWget$DATE.txt
         chmod +x linuxdeploy-x86_64.AppImage
      fi
      #if (! test -x linuxdeploy-plugin-gtk.sh) then
      #   wget -nv "https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh" 2>> logWget$DATE.txt
      #   chmod +x linuxdeploy-plugin-gtk.sh
      #fi
      #pwd
      ./linuxdeploy-x86_64.AppImage -e ../powerBudgetGuiLinux64 --appdir AppDir -i PowerBudget.png -d powerBudget.desktop --output appimage > logLinuxdeploy$DATE.txt
      ret=$?
      file=powerBudget${VER}_${DATE}_Linux_${CPU}_${BIT}bit.AppImage
      if (test "$ret" = "0") then
         mv PowerBudgetGui-x86_64.AppImage ../$file
         cp -a ../$file ../..
         echo "AppImage created: $file"
      else
         echo "AppImage failed: $file"
      fi
   fi
   if (test "$BIT" = "32") then
      echo "As now skip AppImage at 32 bit"
   fi
fi
if (test "$PKG" = "Linux") then
   file=powerBudget${VER}_${DATE}_Linux_${CPU}_${BIT}bit.tgz
   echo "Creating package file:'$file' ..."
   if (test -f $file) then { rm $file ; } fi
   tar -cvaf $file PowerBudget
   mv $file ../..
   echo "Package file:'$file' done"
fi
if (test "$PKG" = "WinMxe" || test "$PKG" = "WinMgw") then
   file=powerBudget${VER}_${DATE}_${PKG}_${BIT}bit.7z
   echo "Creating package file:'$file' ..."
   if (test -f $file) then { rm $file ; } fi
   7z a -m0=lzma -mx=9 -r $file PowerBudget > /dev/null
   mv $file ../..
   echo "Package file:'$file' done"
fi
rm -r AppDir
ret=$?
if (test "$ret" = "0") then
   rm -r PowerBudget
fi
