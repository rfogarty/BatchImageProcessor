#!/bin/bash

if pkg-config --exists opencv4 ; then

   make

else

   if [ ! -z "$CONDA_PREFIX" ] ; then
      echo "Attempting to use Packages from Anaconda environment" 1>&2
      if [ ! -z "$PKG_CONFIG_PATH" ] ; then
         export PKG_CONFIG_PATH="${CONDA_PREFIX}/lib/pkgconfig:$PKG_CONFIG_PATH"
      else
         export PKG_CONFIG_PATH="${CONDA_PREFIX}/lib/pkgconfig"
      fi
   else
      # Hack to configure for USF install
      echo "Attempting to use Packages for OpenCV as installed by USF" 1>&2
      export PKG_CONFIG_PATH=$(readlink -f ../pkgconfig)
   fi
   make
fi

