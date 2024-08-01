#!/bin/sh
VER=`svnversion .`
if [ "x$VER" != x -a "$VER" != exported ]
then
  echo "#define XAVS_VERSION \" svn-$VER\"" >> config.h
  VER=`echo $VER | sed -e 's/[^0-9].*//'`
else
  echo "#define XAVS_VERSION \"\"" >> config.h
  VER="x"
fi
API=`grep '#define XAVS_BUILD' < xavs.h | sed -e 's/.* \([1-9][0-9]*\).*/\1/'`
echo "#define XAVS_POINTVER \"0.$API.$VER\"" >> config.h
