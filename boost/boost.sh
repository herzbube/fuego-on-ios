#===============================================================================
# Filename:  boost.sh
# Author:    Pete Goodliffe
# Copyright: (c) Copyright 2009 Pete Goodliffe
# Licence:   Please feel free to use this, with attribution
#===============================================================================
#
# Builds a Boost XCFramework for the iPhone.
# Creates a set of universal libraries that can be used on an iPhone and in the
# iPhone simulator. Then creates an XCFramework to make using boost in Xcode
# less painful.
#
# To configure the script, define:
#    BOOST_LIBS:        which libraries to build
#    IPHONEOS_BASESDK_VERSION: iPhone SDK version (e.g. 5.1); if left
#                       undefined, the latest SDK known to your Xcode will
#                       will be used
#    IPHONE_SIMULATOR_BASESDK_VERSION: iPhone Simulator SDK version (e.g.
#                       5.1); if left undefined, the latest SDK known to
#                       your Xcode will be used
#    IPHONEOS_DEPLOYMENT_TARGET: iOS deployment target (e.g. 15.0);
#                       if left undefined the default to use is 7.0;
#                       no 32-bit architectures can be built if set to >10.
#    IPHONE_SIMULATOR_DEPLOYMENT_TARGET: iOS simulator deployment target (e.g. 15.0);
#                       if left undefined the default to use is 7.0;
#                       no 32-bit architectures can be built if set to >10.
#    IPHONE_SIMULATOR_ARCHITECTURE_TYPE: Designation of the device build architecture type;
#                       if left undefined the default "arm" is used.
#    IPHONE_ARCHITECTURES: Space separate list of device architectures to build;
#                       if left undefined the default is "armv7 armv7s arm64".
#    IPHONE_SIMULATOR_ARCHITECTURE_TYPE: Designation of the simulator build architecture type;
#                       if left undefined the default "x86" is used.
#    IPHONE_SIMULATOR_ARCHITECTURES: Space separate list of simulator architectures to build;
#                       if left undefined the default is "i386 x86_64".
#
# Then go get the source tar.bz of the boost you want to build, shove it in the
# same directory as this script, and run "./boost.sh". Grab a cuppa. And voila.
#===============================================================================

IPHONEOS_SDKPREFIX="iphoneos"
IPHONE_SIMULATOR_SDKPREFIX="iphonesimulator"

: ${BOOST_LIBS:="thread filesystem program_options system date_time"}
: ${IPHONEOS_BASESDK_VERSION:=`xcrun --sdk $IPHONEOS_SDKPREFIX --show-sdk-version`}
: ${IPHONEOS_DEPLOYMENT_TARGET:=7.0}
: ${IPHONE_SIMULATOR_BASESDK_VERSION:=`xcrun --sdk $IPHONE_SIMULATOR_SDKPREFIX --show-sdk-version`}
: ${IPHONE_SIMULATOR_DEPLOYMENT_TARGET:=7.0}
: ${XCODE_ROOT:=`xcode-select -print-path`}
: ${EXTRA_CPPFLAGS:="-DBOOST_AC_USE_PTHREADS -DBOOST_SP_USE_PTHREADS -std=gnu++98 -stdlib=libc++ -fembed-bitcode"}
: ${IPHONE_ARCHITECTURE_TYPE:="arm"}
: ${IPHONE_ARCHITECTURES:="armv7 armv7s arm64"}
: ${IPHONE_SIMULATOR_ARCHITECTURE_TYPE:="x86"}
: ${IPHONE_SIMULATOR_ARCHITECTURES:="i386 x86_64"}

# The EXTRA_CPPFLAGS definition works around a thread race issue in
# shared_ptr. I encountered this historically and have not verified that
# the fix is no longer required. Without using the posix thread primitives
# an invalid compare-and-swap ARM instruction (non-thread-safe) was used for the
# shared_ptr use count causing nasty and subtle bugs.
#
# Should perhaps also consider/use instead: -BOOST_SP_USE_PTHREADS

: ${SRCDIR:=`pwd`}
: ${IOSBUILDDIR:=`pwd`/ios/build}
: ${PREFIXDIR:=`pwd`/ios/prefix}
: ${FRAMEWORKDIR:=`pwd`/ios/framework}
: ${COMPILER:="clang"}

BOOST_SRC=$SRCDIR/modular-boost

#===============================================================================

# These variables are known to the Apple compiler. At least
# IPHONEOS_DEPLOYMENT_TARGET is, and if it remains exported it will be seen
# by the compiler and cause the bjam build to fail. To prevent this, we
# un-export the variables so that they stay hidden from the compiler.
export -n IPHONEOS_DEPLOYMENT_TARGET
export -n IPHONE_SIMULATOR_DEPLOYMENT_TARGET

IPHONEOS_SDKNAME="${IPHONEOS_SDKPREFIX}${IPHONEOS_BASESDK_VERSION}"
IPHONE_SIMULATOR_SDKNAME="${IPHONE_SIMULATOR_SDKPREFIX}${IPHONE_SIMULATOR_BASESDK_VERSION}"

PLATFORMS_BASEDIR="$XCODE_ROOT/Platforms"
IPHONEOS_PLATFORMDIR="$PLATFORMS_BASEDIR/iPhoneOS.platform"
IPHONE_SIMULATOR_PLATFORMDIR="$PLATFORMS_BASEDIR/iPhoneSimulator.platform"

IPHONEOS_BJAM_TOOLSET="${IPHONEOS_BASESDK_VERSION}~iphone"
IPHONE_SIMULATOR_BJAM_TOOLSET="${IPHONE_SIMULATOR_BASESDK_VERSION}~iphonesim"

IPHONEOS_CPPFLAGS="-miphoneos-version-min=$IPHONEOS_DEPLOYMENT_TARGET $EXTRA_CPPFLAGS"
IPHONE_SIMULATOR_CPPFLAGS="-mios-simulator-version-min=$IPHONE_SIMULATOR_DEPLOYMENT_TARGET $EXTRA_CPPFLAGS"

IPHONE_LIPO="$(xcrun -sdk $IPHONEOS_SDKNAME -find lipo)"
SIM_LIPO="$(xcrun -sdk $IPHONEOS_SDKNAME -find lipo)"
IPHONE_AR="$(xcrun -sdk $IPHONE_SIMULATOR_SDKNAME -find ar)"
SIM_AR="$(xcrun -sdk $IPHONE_SIMULATOR_SDKNAME -find ar)"
IPHONE_LIBTOOL="$(xcrun -sdk $IPHONE_SIMULATOR_SDKNAME -find libtool)"
IPHONE_SIMULATOR_LIBTOOL="$(xcrun -sdk $IPHONE_SIMULATOR_SDKNAME -find libtool)"
IPHONE_COMPILER="$(xcrun -sdk $IPHONE_SIMULATOR_SDKNAME -find $COMPILER)"
SIM_COMPILER="$(xcrun -sdk $IPHONE_SIMULATOR_SDKNAME -find $COMPILER)"

UNIVERSAL_LIBRARY_NAME=libboost.a
# The framework name determines the prefix that consumers must use in their
# include statements, e.g. framework name "boost" results in this statement:
#   #include <boost/someheaderfile.h>
FRAMEWORK_NAME=boost

IPHONE_BUILD_DIR=iphone-build
IPHONE_UNIVERSAL_LIBRARY_PATH=$IOSBUILDDIR/$IPHONE_BUILD_DIR/$UNIVERSAL_LIBRARY_NAME
IPHONE_FRAMEWORK_BUNDLE=$IOSBUILDDIR/$IPHONE_BUILD_DIR/$FRAMEWORK_NAME.framework

IPHONE_SIMULATOR_BUILD_DIR=iphonesim-build
IPHONE_SIMULATOR_UNIVERSAL_LIBRARY_PATH=$IOSBUILDDIR/$IPHONE_SIMULATOR_BUILD_DIR/$UNIVERSAL_LIBRARY_NAME
IPHONE_SIMULATOR_FRAMEWORK_BUNDLE=$IOSBUILDDIR/$IPHONE_SIMULATOR_BUILD_DIR/$FRAMEWORK_NAME.framework

XCFRAMEWORK_BUNDLE=$FRAMEWORKDIR/$FRAMEWORK_NAME.xcframework

#===============================================================================


#===============================================================================
# Functions
#===============================================================================

abort()
{
    echo
    echo "Aborted: $@"
    exit 1
}

doneSection()
{
    echo
    echo "    ================================================================="
    echo "    Done"
    echo
}

#===============================================================================

cleanEverythingReadyToStart()
{
    echo Cleaning everything before we start to build...
    rm -rf $IPHONE_BUILD_DIR $IPHONE_SIMULATOR_BUILD_DIR
    rm -rf $IOSBUILDDIR
    rm -rf $PREFIXDIR
    rm -rf $XCFRAMEWORK_BUNDLE

    # No need to git fetch, we expect that the submodule is up-to-date

    pushd $BOOST_SRC >/dev/null
    # Remove everything not under version control...
    git clean -dfx
    git submodule foreach --recursive git clean -dfx
    # Throw away local changes (i.e. modifications to user-config.jam)
    git reset --hard
    git submodule foreach --recursive git reset --hard
    popd >/dev/null

    doneSection
}

#===============================================================================
updateBoost()
{
    echo Updating boost into $BOOST_SRC...

    IPHONE_COMPILER_ARCHITECTURES_FLAGS=""
    for IPHONE_ARCHITECTURE in $IPHONE_ARCHITECTURES; do
      IPHONE_COMPILER_ARCHITECTURES_FLAGS="$IPHONE_COMPILER_ARCHITECTURES_FLAGS -arch $IPHONE_ARCHITECTURE"
    done
    cat >> $BOOST_SRC/project-config.jam <<EOF
using darwin : $IPHONEOS_BJAM_TOOLSET
   : $IPHONE_COMPILER $IPHONE_COMPILER_ARCHITECTURES_FLAGS -fvisibility=hidden -fvisibility-inlines-hidden $IPHONEOS_CPPFLAGS
   : <striper> <root>$IPHONEOS_PLATFORMDIR/Developer
   : <architecture>$IPHONE_ARCHITECTURE_TYPE <target-os>iphone
   ;
EOF

    IPHONE_SIMULATOR_COMPILER_ARCHITECTURES_FLAGS=""
    for IPHONE_SIMULATOR_ARCHITECTURE in $IPHONE_SIMULATOR_ARCHITECTURES; do
      IPHONE_SIMULATOR_COMPILER_ARCHITECTURES_FLAGS="$IPHONE_SIMULATOR_COMPILER_ARCHITECTURES_FLAGS -arch $IPHONE_SIMULATOR_ARCHITECTURE"
    done
    cat >> $BOOST_SRC/project-config.jam <<EOF
using darwin : $IPHONE_SIMULATOR_BJAM_TOOLSET
   : $SIM_COMPILER $IPHONE_SIMULATOR_COMPILER_ARCHITECTURES_FLAGS -fvisibility=hidden -fvisibility-inlines-hidden $IPHONE_SIMULATOR_CPPFLAGS
   : <striper> <root>$IPHONE_SIMULATOR_PLATFORMDIR/Developer
   : <architecture>$IPHONE_SIMULATOR_ARCHITECTURE_TYPE <target-os>iphone
   ;
EOF

    doneSection
}

#===============================================================================

patchBoost()
{
    echo Patching boost ...

    # Duplicate object file "utf8_codecvt_facet.o" causes build warnings in consuming projects.
    # Renaming source files causes the object files to have unique names.
    # copied from https://github.com/danoli3/ofxiOSBoost/commit/80fe8ef7b71da93d39fafa9a249da51c8d643ab2
    # which in turn copied from http://swift.im/git/swift/tree/3rdParty/Boost/update.sh#n48?id=8dce1cd03729624a25a98dd2c0d026b93e452f86
    echo Fixing utf8_codecvt_facet.cpp duplicates...

    [ -f "$BOOST_SRC/libs/filesystem/src/utf8_codecvt_facet.cpp" ] && (mv "$BOOST_SRC/libs/filesystem/src/utf8_codecvt_facet.cpp" "$BOOST_SRC/libs/filesystem/src/filesystem_utf8_codecvt_facet.cpp" )
    sed -i .bak 's/utf8_codecvt_facet/filesystem_utf8_codecvt_facet/' "$BOOST_SRC/libs/filesystem/build/Jamfile.v2"

    [ -f "$BOOST_SRC/libs/program_options/src/utf8_codecvt_facet.cpp" ] && (mv "$BOOST_SRC/libs/program_options/src/utf8_codecvt_facet.cpp" "$BOOST_SRC/libs/program_options/src/program_options_utf8_codecvt_facet.cpp" )
    sed -i .bak 's/utf8_codecvt_facet/program_options_utf8_codecvt_facet/' "$BOOST_SRC/libs/program_options/build/Jamfile.v2"

    doneSection
}

#===============================================================================

bootstrapBoost()
{
    cd $BOOST_SRC
    BOOST_LIBS_COMMA=$(echo $BOOST_LIBS | sed -e "s/ /,/g")
    echo "Bootstrapping (with libs $BOOST_LIBS_COMMA)"
    ./bootstrap.sh --with-libraries=$BOOST_LIBS_COMMA
    # This is important: Without this step some libraries may not compile
    # because they don't find the necessary headers. Also, without this
    # step the resulting framework bundle will not contain some headers.
    echo "Setting up headers"
    ./b2 headers
    doneSection
}

#===============================================================================

buildBoostForiPhoneOS()
{
    cd $BOOST_SRC

    # Install this one so we can copy the includes for the frameworks...
    ./tools/build/src/engine/bjam -j16 --build-dir=../$IPHONE_BUILD_DIR --stagedir=../$IPHONE_BUILD_DIR/stage --prefix=$PREFIXDIR toolset=darwin-$IPHONEOS_BJAM_TOOLSET architecture=$IPHONE_ARCHITECTURE_TYPE target-os=iphone macosx-version=iphone-${IPHONEOS_BASESDK_VERSION} define=_LITTLE_ENDIAN link=static stage
    ./tools/build/src/engine/bjam -j16 --build-dir=../$IPHONE_BUILD_DIR --stagedir=../$IPHONE_BUILD_DIR/stage --prefix=$PREFIXDIR toolset=darwin-$IPHONEOS_BJAM_TOOLSET architecture=$IPHONE_ARCHITECTURE_TYPE target-os=iphone macosx-version=iphone-${IPHONEOS_BASESDK_VERSION} define=_LITTLE_ENDIAN link=static install
    doneSection

    ./tools/build/src/engine/bjam -j16 --build-dir=../$IPHONE_SIMULATOR_BUILD_DIR --stagedir=../$IPHONE_SIMULATOR_BUILD_DIR/stage --toolset=darwin-$IPHONE_SIMULATOR_BJAM_TOOLSET architecture=$IPHONE_SIMULATOR_ARCHITECTURE_TYPE target-os=iphone macosx-version=iphonesim-${IPHONE_SIMULATOR_BASESDK_VERSION} link=static stage
    doneSection
}

#===============================================================================

scrunchAllLibsTogetherInOneLibPerPlatformPerArchitecture()
{
    cd $SRCDIR

    # lipo -thin aborts with an error if it encounters a non-fat file.
    # Therefore we need special handling if a build was requested for only
    # one architecture.
    unset SINGLE_IPHONE_ARCHITECTURE
    if test "$IPHONE_ARCHITECTURES" = "${IPHONE_ARCHITECTURES/ //}"; then
      SINGLE_IPHONE_ARCHITECTURE=1
    fi
    unset SINGLE_IPHONE_SIMULATOR_ARCHITECTURE
    if test "$IPHONE_SIMULATOR_ARCHITECTURES" = "${IPHONE_SIMULATOR_ARCHITECTURES/ //}"; then
      SINGLE_IPHONE_SIMULATOR_ARCHITECTURE=1
    fi

    echo Splitting all existing fat binaries...
    for NAME in $BOOST_LIBS; do
        LIB_FILENAME="libboost_$NAME.a"
        OBJ_DIRNAME="obj_$NAME"

        echo Decomposing $LIB_FILENAME...

        # Must have separate obj folders for each library, because separate
        # libraries may contain object files with the same name. Also must have
        # separate iOS/simulator folders because the same architecture may be
        # used to build both platforms.

        for ARCHITECTURE in $IPHONE_ARCHITECTURES; do
          mkdir -p $IOSBUILDDIR/$IPHONE_BUILD_DIR/$ARCHITECTURE/$OBJ_DIRNAME
          if test -z "$SINGLE_IPHONE_ARCHITECTURE"; then
            $IPHONE_LIPO "$IPHONE_BUILD_DIR/stage/lib/$LIB_FILENAME" -thin $ARCHITECTURE -o $IOSBUILDDIR/$IPHONE_BUILD_DIR/$ARCHITECTURE/$LIB_FILENAME
          else
            cp "$IPHONE_BUILD_DIR/stage/lib/$LIB_FILENAME" $IOSBUILDDIR/$IPHONE_BUILD_DIR/$ARCHITECTURE/$LIB_FILENAME
          fi
          (cd $IOSBUILDDIR/$IPHONE_BUILD_DIR/$ARCHITECTURE/$OBJ_DIRNAME; $IPHONE_AR -x ../$LIB_FILENAME);
        done

        for ARCHITECTURE in $IPHONE_SIMULATOR_ARCHITECTURES; do
          mkdir -p $IOSBUILDDIR/$IPHONE_SIMULATOR_BUILD_DIR/$ARCHITECTURE/$OBJ_DIRNAME
          if test -z "$SINGLE_IPHONE_SIMULATOR_ARCHITECTURE"; then
            $SIM_LIPO "$IPHONE_SIMULATOR_BUILD_DIR/stage/lib/$LIB_FILENAME" -thin $ARCHITECTURE -o $IOSBUILDDIR/$IPHONE_SIMULATOR_BUILD_DIR/$ARCHITECTURE/$LIB_FILENAME
          else
            cp "$IPHONE_SIMULATOR_BUILD_DIR/stage/lib/$LIB_FILENAME" $IOSBUILDDIR/$IPHONE_SIMULATOR_BUILD_DIR/$ARCHITECTURE/$LIB_FILENAME
          fi
          (cd $IOSBUILDDIR/$IPHONE_SIMULATOR_BUILD_DIR/$ARCHITECTURE/$OBJ_DIRNAME; $SIM_AR -x ../$LIB_FILENAME);
        done
    done

    # The original solution used "ar crus" to create the library for the first
    # architecture, then incrementally add to the library for each subsequent
    # architecture. In certain scenarios, the "ar" utility apparently is not
    # capable of creating archives with mixed CPU types. For instance, when
    # architectures arm64 and x86_64 are built for the simulator, the first
    # invocation of "ar" will create the library with the arm64 object files,
    # but the second invocation of "ar" will silently do nothing, i.e. it will
    # ***NOT*** add the x86_64 object files to the library as expected. Only
    # when "ar" is invoked with the object files of all architectures does the
    # utility print warning/error messages and exit with exit code 1. The
    # messages are
    #   ranlib: archive member: <foo> cputype (16777223) does not match previous archive members cputype (16777228) (all members must match)
    #   [...] repeated for each object file of the architectures beyond the first architecture
    #   ranlib: archive library: <foo> will be fat and ar(1) will not be able to operate on it
    #   ar: internal ranlib command failed
    #
    # The weird thing is that for years "ar crus" worked perfectly to create a
    # universal library with architectures "armv7 armv7s arm64 i386 x86_64"
    # (arm architectures from iOS build, x86 architectures from simulator
    # build). The problem occurred only after switching to Xcode 15 and trying
    # to build the universal library for "arm64 x86_64" (arm64 either from iOS
    # or simulator build, x86_64 always from simulator build). It is not clear
    # whether the problem is due to new behaviour in ar/ranlib from Xcode 15, or
    # whether the problem is tied to the combination of exactly the two
    # architectures arm64 and x86_64.
    #
    # In any case, we now use libtool instead of ar, because libtool does not
    # suffer from the issue. Because libtool does not support adding to an
    # already existing library, the incremental approach was ditched and the
    # library is now created in a single libtool invocation.
    echo "Linking each device build architecture into an universal library => $UNIVERSAL_LIBRARY_NAME"
    rm -f $IPHONE_UNIVERSAL_LIBRARY_PATH
    (cd $IOSBUILDDIR/$IPHONE_BUILD_DIR; $IPHONE_LIBTOOL -static -o $IPHONE_UNIVERSAL_LIBRARY_PATH */obj_*/*.o;)

    echo "Linking each simulator build architecture into an universal library => $UNIVERSAL_LIBRARY_NAME"
    rm -f $IPHONE_SIMULATOR_UNIVERSAL_LIBRARY_PATH
    (cd $IOSBUILDDIR/$IPHONE_SIMULATOR_BUILD_DIR; $IPHONE_SIMULATOR_LIBTOOL -static -o $IPHONE_SIMULATOR_UNIVERSAL_LIBRARY_PATH */obj_*/*.o;)

    doneSection
}

#===============================================================================
buildFramework()
{
    : ${1:?}
    FRAMEWORK_BUNDLE=$1
    UNIVERSAL_LIBRARY_PATH=$2

    VERSION_TYPE=Alpha
    FRAMEWORK_VERSION=A

    FRAMEWORK_CURRENT_VERSION=$BOOST_VERSION
    FRAMEWORK_COMPATIBILITY_VERSION=$BOOST_VERSION

    echo "Framework: Building $(basename $FRAMEWORK_BUNDLE) from $(basename $UNIVERSAL_LIBRARY_PATH)..."

    rm -rf $FRAMEWORK_BUNDLE

    echo "Framework: Setting up directories..."
    mkdir -p $FRAMEWORK_BUNDLE
    mkdir -p $FRAMEWORK_BUNDLE/Versions
    mkdir -p $FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION
    mkdir -p $FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION/Resources
    mkdir -p $FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION/Headers
    mkdir -p $FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION/Documentation

    echo "Framework: Creating symlinks..."
    ln -s $FRAMEWORK_VERSION               $FRAMEWORK_BUNDLE/Versions/Current
    ln -s Versions/Current/Headers         $FRAMEWORK_BUNDLE/Headers
    ln -s Versions/Current/Resources       $FRAMEWORK_BUNDLE/Resources
    ln -s Versions/Current/Documentation   $FRAMEWORK_BUNDLE/Documentation
    ln -s Versions/Current/$FRAMEWORK_NAME $FRAMEWORK_BUNDLE/$FRAMEWORK_NAME

    echo "Framework: Copying universal library..."
    cp $UNIVERSAL_LIBRARY_PATH "$FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION/$FRAMEWORK_NAME"

    echo "Framework: Copying includes..."
    cp -r $PREFIXDIR/include/boost/*  $FRAMEWORK_BUNDLE/Headers/

    echo "Framework: Creating plist..."
    cat > $FRAMEWORK_BUNDLE/Resources/Info.plist <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>English</string>
	<key>CFBundleExecutable</key>
	<string>${FRAMEWORK_NAME}</string>
	<key>CFBundleIdentifier</key>
	<string>org.boost</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundlePackageType</key>
	<string>FMWK</string>
	<key>CFBundleSignature</key>
	<string>????</string>
	<key>CFBundleVersion</key>
	<string>${FRAMEWORK_CURRENT_VERSION}</string>
</dict>
</plist>
EOF
    doneSection
}

#===============================================================================
buildXcFramework()
{
    echo "Building XCFramework..."

    xcodebuild -create-xcframework \
               -framework $IPHONE_FRAMEWORK_BUNDLE \
               -framework $IPHONE_SIMULATOR_FRAMEWORK_BUNDLE \
               -output $XCFRAMEWORK_BUNDLE

    doneSection
}

#===============================================================================
# Execution starts here
#===============================================================================

mkdir -p $IOSBUILDDIR

cleanEverythingReadyToStart

BOOST_VERSION=`cd $BOOST_SRC; git describe --tags | sed -e 's/^.*\/Boost_\([^\/]*\)/\1/'`
echo "BOOST_VERSION:     $BOOST_VERSION"
echo "BOOST_LIBS:        $BOOST_LIBS"
echo "BOOST_SRC:         $BOOST_SRC"
echo "IOSBUILDDIR:       $IOSBUILDDIR"
echo "PREFIXDIR:         $PREFIXDIR"
echo "FRAMEWORKDIR:      $FRAMEWORKDIR"
echo "IPHONEOS_BASESDK_VERSION: $IPHONEOS_BASESDK_VERSION"
echo "IPHONE_SIMULATOR_BASESDK_VERSION: $IPHONE_SIMULATOR_BASESDK_VERSION"
echo "XCODE_ROOT:        $XCODE_ROOT"
echo "COMPILER:          $COMPILER"
echo

bootstrapBoost
updateBoost
patchBoost
buildBoostForiPhoneOS
scrunchAllLibsTogetherInOneLibPerPlatformPerArchitecture
buildFramework $IPHONE_FRAMEWORK_BUNDLE $IPHONE_UNIVERSAL_LIBRARY_PATH
buildFramework $IPHONE_SIMULATOR_FRAMEWORK_BUNDLE $IPHONE_SIMULATOR_UNIVERSAL_LIBRARY_PATH
buildXcFramework

echo "Completed successfully"
echo "The framework is located here: $XCFRAMEWORK_BUNDLE"
echo "Enjoy."

#===============================================================================
