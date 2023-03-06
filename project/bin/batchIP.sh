#!/bin/bash

# Note: this function is a substitute for "readlink -f"
# which may not be supported on non-Gnu Unix derivatives
function portableReadlink() {
   
   TARGET_FILE="$1"
   # If argument is a directory do this...
   if [ -d "${TARGET_FILE}" ] ; then
      pushd "$TARGET_FILE" &> /dev/null
      if [ $? -ne 0 ] ; then
         return 1
      fi
      # Compute the canonicalized name by finding the physical path 
      # for the directory we're in and appending the target file.
      PHYS_DIR="$(pwd -P)"
      RESULT="$PHYS_DIR"
      if [ -e "${RESULT}" ] ; then
         echo "${RESULT}"
         popd &> /dev/null
         return 0
      else
         popd &> /dev/null
         return 1
      fi
   else
      DIR_NAME="$(dirname "$TARGET_FILE")"
      pushd "$DIR_NAME" &> /dev/null
      if [ $? -ne 0 ] ; then
         return 1
      fi
      TARGET_FILE="$(basename "$TARGET_FILE")"

      # Iterate down a (possible) chain of symlinks
      while [ -L "$TARGET_FILE" ]
      do
         TARGET_FILE="$(readlink "$TARGET_FILE")"
         if [ $? -ne 0 ] ; then
            popd &> /dev/null
            return 1
         fi
         cd "$(dirname "$TARGET_FILE")"
         if [ $? -ne 0 ] ; then
            popd &> /dev/null
            return 1
         fi
         TARGET_FILE="$(basename "$TARGET_FILE")"
      done
      PHYS_DIR="$(pwd -P)"
      RESULT="${PHYS_DIR}/${TARGET_FILE}"
      if [ -e "${RESULT}" ] ; then
         echo "${RESULT}"
         popd &> /dev/null
         return 0
      else
         popd &> /dev/null
         return 1
      fi
   fi
}

export -f portableReadlink

######################################################
################# BLOCK TO PHONE HOME ################
# This can be used to find installation directory
# no matter how this script is referenced/linked or
# put in the PATH or whatever

BASENAME=$(basename "$0")
if [ "$0" == "${0#/}" ]
then
    SCRIPTS_RELATIVE_PATH="$PWD"/"${0%/*}"
else
    SCRIPTS_RELATIVE_PATH="${0%/*}"
fi
PATH_TO_EXEC="$(portableReadlink "${SCRIPTS_RELATIVE_PATH}/${BASENAME}")"

# QUALIFIED_PATH is the full path to the script that contains this code.
export QUALIFIED_PATH="$(dirname "${PATH_TO_EXEC}")"

############### END BLOCK TO PHONE HOME ##############
######################################################

if [ ! -z "$CONDA_PREFIX" ] ; then
   if [ ! -z "$LD_LIBRARY_PATH" ] ; then
      export LD_LIBRARY_PATH="$CONDA_PREFIX/lib:$LD_LIBRARY_PATH"
   else
      export LD_LIBRARY_PATH="$CONDA_PREFIX/lib"
   fi
fi

${QUALIFIED_PATH}/batchIP "$@"
