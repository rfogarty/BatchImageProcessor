#!/bin/bash

if pkg-config --exists opencv4 ; then

   make

else

   PKG_CONFIG_PATH=$(readlink -f ../pkgconfig) make

fi

