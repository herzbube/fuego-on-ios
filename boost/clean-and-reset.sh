# This is a helper script that is intended to be executed before building Boost.
# The script will clean the modular-boost subfolder (which is a Git submodule),
# including the subfolders of all of the individual Boost library Git submodules.
#
# Cleaning means removing EVERYTHING that is not under version control, as well
# as discarding ALL local changes. Because this can be dangerous, this cleanup
# is kept separate of the boost.sh build script so that it has to be executed
# explicitly and intentionally.
#
# WARNING: DO NOT EXECUTE THIS SCRIPT IF YOU HAVE CHANGES IN modular-boost THAT
# YOU WANT TO KEEP!

#!/bin/bash

if test ! -d modular-boost; then
  echo modular-boost folder does not exist
  exit 1
fi

pushd modular-boost
if test $? -ne 0; then
  exit 1
fi

# Remove everything not under version control...
git clean -dfx
if test $? -ne 0; then
  exit 1
fi
git submodule foreach --recursive git clean -dfx
if test $? -ne 0; then
  exit 1
fi

# Throw away local changes (in particular modifications to user-config.jam)
git reset --hard
if test $? -ne 0; then
  exit 1
fi
git submodule foreach --recursive git reset --hard
if test $? -ne 0; then
  exit 1
fi

popd >/dev/null

