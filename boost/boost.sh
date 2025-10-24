# Modified version of the script at
# https://github.com/apotocki/boost-iosx/blob/master/scripts/build.sh
# Specifically of the script version in commit 33afef283183a76586b278fb405c51db5840ce10,
# made on 15-Aug-2025:
# https://github.com/apotocki/boost-iosx/blob/33afef283183a76586b278fb405c51db5840ce10/scripts/build.sh
#
# The license for the original script is available in the file
#   boost-software-license-1.0.txt
# that can be found adjacent to this script file.
#
# Modifications made to the original script:
# - Operate on the modular-boost Git repository instead of downloading the tarball of a
#   distributed Boost release.
# - By default build only a small subset of Boost libraries - the ones required to build
#   Fuego. The original script builds all Boost libraries (or at least those hardcoded
#   in the script).
# - Build a single Boost XCFramework that contains all requested Boost libraries. The
#   XCFramework contains a separate fully-fledged sub-framework for each requested
#   combination of platform + architecture, including header files. The original script
#   instead builds one XCFramework per requested Boost library, and the sub-frameworks are
#   simple folders with a single library file but without any headers (the header files
#   are placed into a separate folder outside of any of the XCFramework folders).
#   - Pros
#     - A consuming project only needs to reference a single XCFramework, instead of
#       multiple ones.
#     - The XCFramework is self-contained, i.e. the consuming project does not need to
#       configure an include path to reference header files.
#     - Sub-frameworks contain a proper version in their Info.plist.
#   - Cons
#     - The XCFramework uses a lot more disk space because it contains the headers
#       multiple times.
# - Check whether requested Boost libraries are valid by checking whether a corresponding
#   library folder exists in the filesystem. The original script validates requested Boost
#   libraries by comparing against a hardcoded (and outdated!) list of known libraries.
# - Perform library decomposing and scrunching together into frameworks, as well as
#   XCFramework creation, based on the presence of files/folders in the filesystem after
#   the build is complete. The original script does these things based on environment
#   variable values that are set up at the beginning of script execution.
#   - Pros
#     - No special handling necessary for various Boost libraries that when built result
#       in more than one library file (e.g. the "math" library results in
#       libboost_math_c99.a, libboost_math_c99f.a, libboost_math_c99l.a, etc.).
#   - Cons
#     - Somewhat error prone, in case there are leftovers from a previous build.
# - When invoking b2, don't use -j8 option, because B2_BUILD_OPTIONS already contains the
#   -j option (supplied value is $THREAD_COUNT, which is based on sysctl).
# - BUILD_DIR is a subfolder located adjacent to this script. The original script sets
#   BUILD_DIR to be the current working directory.
# - ICU building is disabled.
# - Version variables (e.g. IOS_VERSION, IOS_SIM_VERSION, etc.)
#   - The definitions are made so that if the environment variable is already set by the
#     caller, the value provided by the caller is used. The original script does not allow
#     to override the hardcoded values.
#   - IOS_VERSION and IOS_SIM_VERSION constitute deployment target versions. The defaults
#     values for these variables are set to 12.0, which are the oldest version still
#     supported by the iOS 26 SDK that is used to modernize the Boost build. On a
#     side-note, 11.0 was the first version that no longer supports 32-bit builds.
#     The original script has 13.4 as hardcoded default value for both variables.
# - Add some compiler options to B2_BUILD_OPTIONS to ensure compatibility with Little Go.
#     -stdlib=libc++
#     -fvisibility=hidden
#     -fvisibility-inlines-hidden
#
# The way how the individual Boost libraries' library files are scrunched together into
# a single library per platform + architecture was merged into the original script from
# code that existed in a previous version of this file - see the Git history. That
# previous version was inspired by Pete Goodcliffe's original "build Boost for iOS"
# script, with various modifications. See the repository's top-level README.md for
# details.
#
# Things that were not taken over from a previous version of this script:
# - Generation of header files with "./b2 headers". It seems that this is done
#   automatically by the Boost build. The header files can be found in
#   modular-boost/boost.
# - The ability to specify base SDK versions via the environment variables
#     IPHONEOS_BASESDK_VERSION
#     IPHONE_SIMULATOR_BASESDK_VERSION
#   The new version of the script does not contain support for defining the base SDK,
#   and because this flexibility was never used in the many years since this repository
#   was conceived, it did not seem worth the extra effort to re-develop the capability.
# - The following compiler options:
#     -DBOOST_AC_USE_PTHREADS
#     -DBOOST_SP_USE_PTHREADS
#   There was a remark by Pete Goodcliffe in the previous version of this script that said
#   that these were required at one time to work around a thread race issue in shared_ptr,
#   but it was not clear whether this workaround was still required.
# - The compiler option -std=gnu++98. This no longer works with a modern Boost version.

#!/bin/bash
set -euo pipefail
################## SETUP BEGIN
THREAD_COUNT=$(sysctl hw.ncpu | awk '{print $2}')
HOST_ARC=$( uname -m )
XCODE_ROOT=$( xcode-select -print-path )
# - MACOSX_VERSION_ARM, MACOSX_VERSION_X86_64, IOS_VERSION and IOS_SIM_VERSION are used to
#   set the respective deployment targets.
# - TVOS_VERSION and WATCHOS_VERSION are currently not used
# - CATALYST_VERSION, TVOS_SIM_VERSION and WATCHOS_SIM_VERSION are used to form the value
#   of the --target compiler option of the respective builds.
: ${MACOSX_VERSION_ARM:=12.3}
: ${MACOSX_VERSION_X86_64:=10.13}
: ${IOS_VERSION:=12.0}
: ${IOS_SIM_VERSION:=12.0}
: ${CATALYST_VERSION:=13.4}
: ${TVOS_VERSION:=13.0}
: ${TVOS_SIM_VERSION:=13.0}
: ${WATCHOS_VERSION:=11.0}
: ${WATCHOS_SIM_VERSION:=11.0}
################## SETUP END
IOSSYSROOT=$XCODE_ROOT/Platforms/iPhoneOS.platform/Developer
IOSSIMSYSROOT=$XCODE_ROOT/Platforms/iPhoneSimulator.platform/Developer
MACSYSROOT=$XCODE_ROOT/Platforms/MacOSX.platform/Developer
XROSSYSROOT=$XCODE_ROOT/Platforms/XROS.platform/Developer
XROSSIMSYSROOT=$XCODE_ROOT/Platforms/XRSimulator.platform/Developer
TVOSSYSROOT=$XCODE_ROOT/Platforms/AppleTVOS.platform/Developer
TVOSSIMSYSROOT=$XCODE_ROOT/Platforms/AppleTVSimulator.platform/Developer
WATCHOSSYSROOT=$XCODE_ROOT/Platforms/WatchOS.platform/Developer
WATCHOSSIMSYSROOT=$XCODE_ROOT/Platforms/WatchSimulator.platform/Developer

BUILD_PLATFORMS_ALL="macosx,macosx-arm64,macosx-x86_64,macosx-both,ios,iossim,iossim-arm64,iossim-x86_64,iossim-both,catalyst,catalyst-arm64,catalyst-x86_64,catalyst-both,xros,xrossim,xrossim-arm64,xrossim-x86_64,xrossim-both,tvos,tvossim,tvossim-both,tvossim-arm64,tvossim-x86_64,watchos,watchossim,watchossim-both,watchossim-arm64,watchossim-x86_64"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BOOST_DIR="$SCRIPT_DIR/modular-boost"
BOOST_HEADERS_DIR="$BOOST_DIR/boost"
BUILD_DIR="$SCRIPT_DIR/build"
UNIVERSAL_LIBRARY_NAME=libboost.a
# The framework name determines the prefix that consumers must use in their
# include statements, e.g. framework name "boost" results in this statement:
#   #include <boost/someheaderfile.h>
FRAMEWORK_NAME=boost
FRAMEWORK_BUNDLE_NAME=$FRAMEWORK_NAME.framework
XCFRAMEWORK_BUNDLE_NAME=$FRAMEWORK_NAME.xcframework
# A previous version of this script used the -sdk parameter to locate platform-specific
# versions of each tool. This was removed because it seemed to be unnecessary overhead,
# as these tools merely need to run on the build system.
LIPO="$(xcrun -find lipo)"
AR="$(xcrun -find ar)"
LIBTOOL="$(xcrun -find libtool)"
BOOST_VERSION=`cd $BOOST_DIR; git describe --tags | sed -e 's/^.*\/Boost_\([^\/]*\)/\1/'`

[[ $(clang++ --version | head -1 | sed -E 's/([a-zA-Z ]+)([0-9]+).*/\2/') -gt 14 ]] && CLANG15=true

LIBS_TO_BUILD="thread,filesystem,program_options,system,date_time"
[[ ! $CLANG15 ]] && LIBS_TO_BUILD="${LIBS_TO_BUILD/,cobalt/}"

BUILD_PLATFORMS="macosx,ios,iossim,catalyst"
[[ -d $XROSSYSROOT/SDKs/XROS.sdk ]] && BUILD_PLATFORMS="$BUILD_PLATFORMS,xros"
[[ -d $XROSSIMSYSROOT/SDKs/XRSimulator.sdk ]] && BUILD_PLATFORMS="$BUILD_PLATFORMS,xrossim"
[[ -d $TVOSSYSROOT/SDKs/AppleTVOS.sdk ]] && BUILD_PLATFORMS="$BUILD_PLATFORMS,tvos"
[[ -d $TVOSSIMSYSROOT/SDKs/AppleTVSimulator.sdk ]] && BUILD_PLATFORMS="$BUILD_PLATFORMS,tvossim"
[[ -d $WATCHOSSYSROOT/SDKs/WatchOS.sdk ]] && BUILD_PLATFORMS="$BUILD_PLATFORMS,watchos"
[[ -d $WATCHOSSIMSYSROOT/SDKs/WatchSimulator.sdk ]] && BUILD_PLATFORMS="$BUILD_PLATFORMS,watchossim-both"

REBUILD=false

# Function to determine architecture
boost_arc() {
    case $1 in
        arm*) echo "arm" ;;
        x86*) echo "x86" ;;
        *) echo "unknown" ;;
    esac
}

# Function to determine ABI
boost_abi() {
    case $1 in
        arm64) echo "aapcs" ;;
        x86_64) echo "sysv" ;;
        *) echo "unknown" ;;
    esac
}

is_subset() {
    local mainset=($(< $1))
    shift
    local subset=("$@")
    
    for element in "${subset[@]}"; do
        if [[ ! " ${mainset[@]} " =~ " ${element} " ]]; then
            echo "false"
            return
        fi
    done
    echo "true"
}

# Parse command line arguments
for i in "$@"; do
  case $i in
    -l=*|--libs=*)
      LIBS_TO_BUILD="${i#*=}"
      shift
      ;;
    -p=*|--platforms=*)
      BUILD_PLATFORMS="${i#*=},"
      shift
      ;;
    --rebuild)
      REBUILD=true
      [[ -f "$BUILD_DIR/frameworks.built.platforms" ]] && rm "$BUILD_DIR/frameworks.built.platforms"
      [[ -f "$BUILD_DIR/frameworks.built.libs" ]] && rm "$BUILD_DIR/frameworks.built.libs"
      shift
      ;;
    --rebuildicu)
      [[ -d $SCRIPT_DIR/Pods/icu4c-iosx ]] && rm -rf $SCRIPT_DIR/Pods/icu4c-iosx
      shift
      ;;
    -*|--*)
      echo "Unknown option $i"
      exit 1
      ;;
    *)
      ;;
  esac
done

LIBS_TO_BUILD=${LIBS_TO_BUILD//,/ }

#sort the library list
LIBS_TO_BUILD_ARRAY=($LIBS_TO_BUILD)
IFS=$'\n' LIBS_TO_BUILD_SORTED_ARRAY=($(sort <<<"${LIBS_TO_BUILD_ARRAY[*]}")); unset IFS
LIBS_TO_BUILD_SORTED="${LIBS_TO_BUILD_SORTED_ARRAY[@]}"
#LIBS_HASH=$( echo -n $LIBS_TO_BUILD_SORTED | shasum -a 256 | awk '{ print $1 }' )

for i in $LIBS_TO_BUILD; do :;
if [[ ! -d "$BOOST_DIR/libs/$i" ]]; then
	echo "Unknown library '$i'"
	exit 1
fi
done

[[ $BUILD_PLATFORMS == *macosx-both* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//macosx-both/},macosx-arm64,macosx-x86_64"
[[ $BUILD_PLATFORMS == *iossim-both* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//iossim-both/},iossim-arm64,iossim-x86_64"
[[ $BUILD_PLATFORMS == *catalyst-both* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//catalyst-both/},catalyst-arm64,catalyst-x86_64"
[[ $BUILD_PLATFORMS == *xrossim-both* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//xrossim-both/},xrossim-arm64,xrossim-x86_64"
[[ $BUILD_PLATFORMS == *tvossim-both* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//tvossim-both/},tvossim-arm64,tvossim-x86_64"
[[ $BUILD_PLATFORMS == *watchossim-both* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//watchossim-both/},watchossim-arm64,watchossim-x86_64"
BUILD_PLATFORMS="$BUILD_PLATFORMS,"
[[ $BUILD_PLATFORMS == *"macosx,"* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//macosx,/,},macosx-$HOST_ARC"
[[ $BUILD_PLATFORMS == *"iossim,"* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//iossim,/,},iossim-$HOST_ARC"
[[ $BUILD_PLATFORMS == *"catalyst,"* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//catalyst,/,},catalyst-$HOST_ARC"
[[ $BUILD_PLATFORMS == *"xrossim,"* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//xrossim,/,},xrossim-$HOST_ARC"
[[ $BUILD_PLATFORMS == *"tvossim,"* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//tvossim,/,},tvossim-$HOST_ARC"
[[ $BUILD_PLATFORMS == *"watchossim,"* ]] && BUILD_PLATFORMS="${BUILD_PLATFORMS//watchossim,/,},watchossim-$HOST_ARC"

if [[ $BUILD_PLATFORMS == *"xros,"* ]] && [[ ! -d $XROSSYSROOT/SDKs/XROS.sdk ]]; then
    echo "The xros is specified as the build platform, but XROS.sdk is not found (the path $XROSSYSROOT/SDKs/XROS.sdk)."
    exit 1
fi

if [[ $BUILD_PLATFORMS == *"xrossim"* ]] && [[ ! -d $XROSSIMSYSROOT/SDKs/XRSimulator.sdk ]]; then
    echo "The xrossim is specified as the build platform, but XRSimulator.sdk is not found (the path $XROSSIMSYSROOT/SDKs/XRSimulator.sdk)."
    exit 1
fi

if [[ $BUILD_PLATFORMS == *"tvos,"* ]] && [[ ! -d $TVOSSYSROOT/SDKs/AppleTVOS.sdk ]]; then
    echo "The tvos is specified as the build platform, but AppleTVOS.sdk is not found (the path $TVOSSYSROOT/SDKs/AppleTVOS.sdk)."
    exit 1
fi

if [[ $BUILD_PLATFORMS == *"tvossim"* ]] && [[ ! -d $TVOSSIMSYSROOT/SDKs/AppleTVSimulator.sdk ]]; then
    echo "The tvossim is specified as the build platform, but AppleTVSimulator.sdk is not found (the path $TVOSSIMSYSROOT/SDKs/AppleTVSimulator.sdk)."
    exit 1
fi

if [[ $BUILD_PLATFORMS == *"watchos,"* ]] && [[ ! -d $WATCHOSSYSROOT/SDKs/WatchOS.sdk ]]; then
    echo "The tvos is specified as the build platform, but WatchOS.sdk is not found (the path $WATCHOSSYSROOT/SDKs/WatchOS.sdk)."
    exit 1
fi

if [[ $BUILD_PLATFORMS == *"watchossim"* ]] && [[ ! -d $WATCHOSSIMSYSROOT/SDKs/WatchSimulator.sdk ]]; then
    echo "The tvos is specified as the build platform, but WatchSimulator.sdk is not found (the path $WATCHOSSIMSYSROOT/SDKs/WatchSimulator.sdk)."
    exit 1
fi

BUILD_PLATFORMS_SPACED=" ${BUILD_PLATFORMS//,/ } "
BUILD_PLATFORMS_ARRAY=($BUILD_PLATFORMS_SPACED)

for i in $BUILD_PLATFORMS_SPACED; do :;
if [[ ! ",$BUILD_PLATFORMS_ALL," == *",$i,"* ]]; then
	echo "Unknown platform '$i'"
	exit 1
fi
done

if test ! -d "$BUILD_DIR"; then
  mkdir -p "$BUILD_DIR"
fi

[[ -f "$BUILD_DIR/frameworks.built.platforms" ]] && [[ -f "$BUILD_DIR/frameworks.built.libs" ]] && [[ $(< $BUILD_DIR/frameworks.built.platforms) == $BUILD_PLATFORMS ]] && [[ $(< $BUILD_DIR/frameworks.built.libs) == $LIBS_TO_BUILD ]] && exit 0

[[ -f "$BUILD_DIR/frameworks.built.platforms" ]] && rm "$BUILD_DIR/frameworks.built.platforms"
[[ -f "$BUILD_DIR/frameworks.built.libs" ]] && rm "$BUILD_DIR/frameworks.built.libs"

if [[ ! -f $BOOST_DIR/b2 ]]; then
	pushd "$BOOST_DIR"
	./bootstrap.sh
	popd
fi

############### ICU
if false; then
#export ICU4C_RELEASE_LINK=https://github.com/apotocki/icu4c-iosx/releases/download/76.1.4
if [[ ! -f $SCRIPT_DIR/Pods/icu4c-iosx/build.success ]] || [[ $(is_subset $SCRIPT_DIR/Pods/icu4c-iosx/build.success "${BUILD_PLATFORMS_ARRAY[@]}") == "false" ]]; then
    if [[ ! -z "${ICU4C_RELEASE_LINK:-}" ]]; then
		[[ -d $SCRIPT_DIR/Pods/icu4c-iosx ]] && rm -rf $SCRIPT_DIR/Pods/icu4c-iosx
		mkdir -p $SCRIPT_DIR/Pods/icu4c-iosx/product
		pushd $SCRIPT_DIR/Pods/icu4c-iosx/product
        curl -L ${ICU4C_RELEASE_LINK}/include.zip -o $SCRIPT_DIR/Pods/icu4c-iosx/product/include.zip
		curl -L ${ICU4C_RELEASE_LINK}/icudata.xcframework.zip -o $SCRIPT_DIR/Pods/icu4c-iosx/product/icudata.xcframework.zip
		curl -L ${ICU4C_RELEASE_LINK}/icui18n.xcframework.zip -o $SCRIPT_DIR/Pods/icu4c-iosx/product/icui18n.xcframework.zip
        #curl -L ${ICU4C_RELEASE_LINK}/icuio.xcframework.zip -o $SCRIPT_DIR/Pods/icu4c-iosx/product/icuio.xcframework.zip
        curl -L ${ICU4C_RELEASE_LINK}/icuuc.xcframework.zip -o $SCRIPT_DIR/Pods/icu4c-iosx/product/icuuc.xcframework.zip
		unzip -q include.zip
		unzip -q icudata.xcframework.zip
		unzip -q icui18n.xcframework.zip
        #unzip -q icuio.xcframework.zip
        unzip -q icuuc.xcframework.zip
		mkdir frameworks
		mv icudata.xcframework frameworks/
		mv icui18n.xcframework frameworks/
        #mv icuio.xcframework frameworks/
        mv icuuc.xcframework frameworks/
        popd
        printf "${BUILD_PLATFORMS_ALL//,/ }" > build.success
    else
        if [[ ! -f $SCRIPT_DIR/Pods/icu4c-iosx/everbuilt.success ]]; then
            [[ -d $SCRIPT_DIR/Pods/icu4c-iosx ]] && rm -rf $SCRIPT_DIR/Pods/icu4c-iosx
            [[ ! -d $SCRIPT_DIR/Pods ]] && mkdir $SCRIPT_DIR/Pods
            pushd $SCRIPT_DIR/Pods
            git clone https://github.com/apotocki/icu4c-iosx
        else
            pushd $SCRIPT_DIR/Pods/icu4c-iosx
            git pull
        fi
        popd
        
        pushd $SCRIPT_DIR/Pods/icu4c-iosx
        scripts/build.sh -p=$BUILD_PLATFORMS
        touch everbuilt.success
        printf "${BUILD_PLATFORMS//,/ }" > build.success
        popd
        
        #pushd $SCRIPT_DIR
        #pod repo update
        #pod install --verbose
        ##pod update --verbose
        #popd
    fi
    mkdir -p $SCRIPT_DIR/Pods/icu4c-iosx/product/lib
fi
ICU_PATH=$SCRIPT_DIR/Pods/icu4c-iosx/product
fi
############### ICU

pushd "$BOOST_DIR"

echo patching boost...


#if [ ! -f boost/json/impl/array.ipp.orig ]; then
#	cp -f boost/json/impl/array.ipp boost/json/impl/array.ipp.orig
#else
#	cp -f boost/json/impl/array.ipp.orig boost/json/impl/array.ipp
#fi
#if [ ! -f libs/json/test/array.cpp.orig ]; then
#	cp -f libs/json/test/array.cpp libs/json/test/array.cpp.orig
#else
#	cp -f libs/json/test/array.cpp.orig libs/json/test/array.cpp
#fi
#patch -p0 <$SCRIPT_DIR/0001-json-array-erase-relocate.patch

if [[ ! -f tools/build/src/tools/features/instruction-set-feature.jam.orig ]]; then
	cp -f tools/build/src/tools/features/instruction-set-feature.jam tools/build/src/tools/features/instruction-set-feature.jam.orig
else
	cp -f tools/build/src/tools/features/instruction-set-feature.jam.orig tools/build/src/tools/features/instruction-set-feature.jam
fi
patch tools/build/src/tools/features/instruction-set-feature.jam $SCRIPT_DIR/instruction-set-feature.jam.patch

# TODO xxx add patching for utf8_codecvt_facet

B2_BUILD_OPTIONS="-j$THREAD_COUNT address-model=64 release link=static runtime-link=shared define=BOOST_SPIRIT_THREADSAFE cxxflags=\"-std=c++20\""
B2_BUILD_OPTIONS="$B2_BUILD_OPTIONS cxxflags=\"-stdlib=libc++\" cxxflags=\"-fvisibility=hidden\" cxxflags=\"-fvisibility-inlines-hidden\""

[[ ! -z "${ICU_PATH:-}" ]] && B2_BUILD_OPTIONS="$B2_BUILD_OPTIONS -sICU_PATH=\"$ICU_PATH\""

for i in $LIBS_TO_BUILD; do :;
  B2_BUILD_OPTIONS="$B2_BUILD_OPTIONS --with-$i"
done

[[ -d bin.v2 ]] && rm -rf bin.v2


#(paltform=$1 architecture=$2 additional_flags=$3 root=$4 depfilter=$5 additional_config=$6 additional_b2flags=$7)
build_generic_libs()
{
if [[ $REBUILD == true ]] || [[ ! -f $1-$2-build.success ]] || [[ $(is_subset $1-$2-build.success "${LIBS_TO_BUILD_ARRAY[@]}") == "false" ]]; then
    [[ -f $1-$2-build.success ]] && rm $1-$2-build.success
    
    [[ -f tools/build/src/user-config.jam ]] && rm -f tools/build/src/user-config.jam

    cat >> tools/build/src/user-config.jam <<EOF
using darwin : $1 : clang++ -arch $2 $3
: <striper> <root>$4
: <architecture>$(boost_arc $2) ${6:-}
;
EOF
    if [[ ! -z "${ICU_PATH:-}" ]]; then
        cp $ICU_PATH/frameworks/icudata.xcframework/$5/libicudata.a $ICU_PATH/lib/
        cp $ICU_PATH/frameworks/icui18n.xcframework/$5/libicui18n.a $ICU_PATH/lib/
        cp $ICU_PATH/frameworks/icuuc.xcframework/$5/libicuuc.a $ICU_PATH/lib/
    fi

    ./b2 --stagedir=stage/$1-$2 toolset=darwin-$1 architecture=$(boost_arc $2) abi=$(boost_abi $2) ${7:-} $B2_BUILD_OPTIONS
    rm -rf bin.v2
    printf "$LIBS_TO_BUILD_SORTED" > $1-$2-build.success
fi
}

build_macos_libs()
{
    build_generic_libs macosx $1 "$2 -isysroot $MACSYSROOT/SDKs/MacOSX.sdk" $MACSYSROOT "macos-*"
}

build_catalyst_libs()
{
    build_generic_libs catalyst $1 "--target=$1-apple-ios$CATALYST_VERSION-macabi -isysroot $MACSYSROOT/SDKs/MacOSX.sdk -I$MACSYSROOT/SDKs/MacOSX.sdk/System/iOSSupport/usr/include/ -isystem $MACSYSROOT/SDKs/MacOSX.sdk/System/iOSSupport/usr/include -iframework $MACSYSROOT/SDKs/MacOSX.sdk/System/iOSSupport/System/Library/Frameworks" $MACSYSROOT "ios-*-maccatalyst"
}

build_ios_libs()
{
    build_generic_libs ios arm64 "-fembed-bitcode -isysroot $IOSSYSROOT/SDKs/iPhoneOS.sdk -mios-version-min=$IOS_VERSION" $IOSSYSROOT "ios-arm64" "<target-os>iphone" "instruction-set=arm64 binary-format=mach-o target-os=iphone define=_LITTLE_ENDIAN define=BOOST_TEST_NO_MAIN"
}

build_xros_libs()
{
    build_generic_libs xros arm64 "-fembed-bitcode -isysroot $XROSSYSROOT/SDKs/XROS.sdk" $XROSSYSROOT "xros-arm64" "<target-os>iphone" "instruction-set=arm64 binary-format=mach-o target-os=iphone define=_LITTLE_ENDIAN define=BOOST_TEST_NO_MAIN"
}

build_tvos_libs()
{
    build_generic_libs tvos arm64 "-fembed-bitcode -isysroot $TVOSSYSROOT/SDKs/AppleTVOS.sdk" $TVOSSYSROOT "tvos-arm64" "<target-os>iphone" "instruction-set=arm64 binary-format=mach-o target-os=iphone define=_LITTLE_ENDIAN define=BOOST_TEST_NO_MAIN define=BOOST_TEST_DISABLE_ALT_STACK"
}

build_watchos_libs()
{
    build_generic_libs watchos arm64 "-fembed-bitcode -isysroot $WATCHOSSYSROOT/SDKs/WatchOS.sdk" $WATCHOSSYSROOT "watchos-arm64" "<target-os>iphone" "instruction-set=arm64 binary-format=mach-o target-os=iphone define=_LITTLE_ENDIAN define=BOOST_TEST_NO_MAIN define=BOOST_TEST_DISABLE_ALT_STACK"
}

build_sim_libs()
{
    build_generic_libs iossim $1 "-mios-simulator-version-min=$IOS_SIM_VERSION -isysroot $IOSSIMSYSROOT/SDKs/iPhoneSimulator.sdk" $IOSSIMSYSROOT "ios-*-simulator" "<target-os>iphone" "target-os=iphone define=BOOST_TEST_NO_MAIN"
}

build_xrossim_libs()
{
    build_generic_libs xrossim $1 "-isysroot $XROSSIMSYSROOT/SDKs/XRSimulator.sdk" $XROSSIMSYSROOT "xros-*-simulator" "<target-os>iphone" "target-os=iphone define=BOOST_TEST_NO_MAIN"
}

build_tvossim_libs()
{
    build_generic_libs tvossim $1 " --target=$1-apple-tvos$TVOS_SIM_VERSION-simulator -isysroot $TVOSSIMSYSROOT/SDKs/AppleTVSimulator.sdk" $TVOSSIMSYSROOT "tvos-*-simulator" "<target-os>iphone" "target-os=iphone define=BOOST_TEST_NO_MAIN define=BOOST_TEST_DISABLE_ALT_STACK"
}

build_watchossim_libs()
{
    build_generic_libs watchossim $1 "--target=$1-apple-watchos$WATCHOS_SIM_VERSION-simulator -isysroot $WATCHOSSIMSYSROOT/SDKs/WatchSimulator.sdk" $WATCHOSSIMSYSROOT "watchos-*-simulator" "<target-os>iphone" "target-os=iphone define=BOOST_TEST_NO_MAIN define=BOOST_TEST_DISABLE_ALT_STACK"
}

[[ -d stage/macosx/lib ]] && rm -rf stage/macosx/lib
[[ "$BUILD_PLATFORMS_SPACED" == *"macosx-arm64"* ]] && build_macos_libs arm64 -mmacosx-version-min=$MACOSX_VERSION_ARM
[[ "$BUILD_PLATFORMS_SPACED" == *"macosx-x86_64"* ]] && build_macos_libs x86_64 -mmacosx-version-min=$MACOSX_VERSION_X86_64
[[ "$BUILD_PLATFORMS_SPACED" == *"macosx"* ]] && mkdir -p stage/macosx/lib

[ -d stage/catalyst/lib ] && rm -rf stage/catalyst/lib
[[ "$BUILD_PLATFORMS_SPACED" == *"catalyst-arm64"* ]] && build_catalyst_libs arm64
[[ "$BUILD_PLATFORMS_SPACED" == *"catalyst-x86_64"* ]] && build_catalyst_libs x86_64
[[ "$BUILD_PLATFORMS_SPACED" == *"catalyst"* ]] && mkdir -p stage/catalyst/lib

[ -d stage/iossim/lib ] && rm -rf stage/iossim/lib
[[ "$BUILD_PLATFORMS_SPACED" == *"iossim-arm64"* ]] && build_sim_libs arm64
[[ "$BUILD_PLATFORMS_SPACED" == *"iossim-x86_64"* ]] && build_sim_libs x86_64
[[ "$BUILD_PLATFORMS_SPACED" == *"iossim"* ]] && mkdir -p stage/iossim/lib

[ -d stage/xrossim/lib ] && rm -rf stage/xrossim/lib
[[ "$BUILD_PLATFORMS_SPACED" == *"xrossim-arm64"* ]] && build_xrossim_libs arm64
[[ "$BUILD_PLATFORMS_SPACED" == *"xrossim-x86_64"* ]] && build_xrossim_libs x86_64
[[ "$BUILD_PLATFORMS_SPACED" == *"xrossim"* ]] && mkdir -p stage/xrossim/lib

[ -d stage/tvossim/lib ] && rm -rf stage/tvossim/lib
[[ "$BUILD_PLATFORMS_SPACED" == *"tvossim-arm64"* ]] && build_tvossim_libs arm64
[[ "$BUILD_PLATFORMS_SPACED" == *"tvossim-x86_64"* ]] && build_tvossim_libs x86_64
[[ "$BUILD_PLATFORMS_SPACED" == *"tvossim"* ]] && mkdir -p stage/tvossim/lib

[ -d stage/watchossim/lib ] && rm -rf stage/watchossim/lib
[[ "$BUILD_PLATFORMS_SPACED" == *"watchossim-arm64"* ]] && build_watchossim_libs arm64
[[ "$BUILD_PLATFORMS_SPACED" == *"watchossim-x86_64"* ]] && build_watchossim_libs x86_64
[[ "$BUILD_PLATFORMS_SPACED" == *"watchossim"* ]] && mkdir -p stage/watchossim/lib

[[ "$BUILD_PLATFORMS_SPACED" == *"ios "* ]] && build_ios_libs
[[ "$BUILD_PLATFORMS_SPACED" == *"xros "* ]] && build_xros_libs
[[ "$BUILD_PLATFORMS_SPACED" == *"tvos "* ]] && build_tvos_libs
[[ "$BUILD_PLATFORMS_SPACED" == *"watchos "* ]] && build_watchos_libs

echo installing boost...
[[ -d "$BUILD_DIR/frameworks" ]] && rm -rf "$BUILD_DIR/frameworks"
mkdir "$BUILD_DIR/frameworks"

buildFramework()
{
    : ${1:?}
    FRAMEWORK_BUNDLE_PATH=$1
    UNIVERSAL_LIBRARY_PATH=$2

    VERSION_TYPE=Alpha
    FRAMEWORK_VERSION=A

    FRAMEWORK_CURRENT_VERSION=$BOOST_VERSION
    FRAMEWORK_COMPATIBILITY_VERSION=$BOOST_VERSION

    echo "Framework: Building $(basename $FRAMEWORK_BUNDLE_PATH) from $(basename $UNIVERSAL_LIBRARY_PATH)..."

    rm -rf $FRAMEWORK_BUNDLE_PATH

    echo "Framework: Setting up directories..."
    mkdir -p $FRAMEWORK_BUNDLE_PATH
    mkdir -p $FRAMEWORK_BUNDLE_PATH/Versions
    mkdir -p $FRAMEWORK_BUNDLE_PATH/Versions/$FRAMEWORK_VERSION
    mkdir -p $FRAMEWORK_BUNDLE_PATH/Versions/$FRAMEWORK_VERSION/Resources
    mkdir -p $FRAMEWORK_BUNDLE_PATH/Versions/$FRAMEWORK_VERSION/Headers
    mkdir -p $FRAMEWORK_BUNDLE_PATH/Versions/$FRAMEWORK_VERSION/Documentation

    echo "Framework: Creating symlinks..."
    ln -s $FRAMEWORK_VERSION               $FRAMEWORK_BUNDLE_PATH/Versions/Current
    ln -s Versions/Current/Headers         $FRAMEWORK_BUNDLE_PATH/Headers
    ln -s Versions/Current/Resources       $FRAMEWORK_BUNDLE_PATH/Resources
    ln -s Versions/Current/Documentation   $FRAMEWORK_BUNDLE_PATH/Documentation
    ln -s Versions/Current/$FRAMEWORK_NAME $FRAMEWORK_BUNDLE_PATH/$FRAMEWORK_NAME

    echo "Framework: Copying universal library..."
    cp $UNIVERSAL_LIBRARY_PATH "$FRAMEWORK_BUNDLE_PATH/Versions/$FRAMEWORK_VERSION/$FRAMEWORK_NAME"

    echo "Framework: Copying includes..."
    cp -RL $BOOST_HEADERS_DIR/*  $FRAMEWORK_BUNDLE_PATH/Headers/

    echo "Framework: Creating plist..."
    cat > $FRAMEWORK_BUNDLE_PATH/Resources/Info.plist <<EOF
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
}

scrunchAllLibsTogetherInOneLibPerPlatformPerArchitecture()
{
  for PLATFORM_ARCHITECTURE_COMBO_STAGE_PATH in stage/*; do
    PLATFORM_ARCHITECTURE_COMBO_NAME="$(basename $PLATFORM_ARCHITECTURE_COMBO_STAGE_PATH)"
    PLATFORM_ARCHITECTURE_COMBO_BUILD_PATH="$BUILD_DIR/$PLATFORM_ARCHITECTURE_COMBO_NAME"
    
    UNIVERSAL_LIBRARY_PATH="$PLATFORM_ARCHITECTURE_COMBO_BUILD_PATH/$UNIVERSAL_LIBRARY_NAME"
    FRAMEWORK_BUNDLE_PATH="$PLATFORM_ARCHITECTURE_COMBO_BUILD_PATH/$FRAMEWORK_BUNDLE_NAME"

    # For some pseudo platforms (e.g. iossim, macosx) a staging folder with an empty
    # "libs" subfolder is created => this we can skip
    NUMBER_OF_LIBARIES_TO_DECOMPOSE=$(ls $PLATFORM_ARCHITECTURE_COMBO_STAGE_PATH/lib | wc -l)
    if test "$NUMBER_OF_LIBARIES_TO_DECOMPOSE" -eq 0; then
      continue
    fi

    echo "Decomposing all libraries for $PLATFORM_ARCHITECTURE_COMBO_NAME..."
    for LIBRARY_PATH in $PLATFORM_ARCHITECTURE_COMBO_STAGE_PATH/lib/*.a; do
      LIBRARY_FILENAME="$(basename $LIBRARY_PATH)"
      LIBRARY_NAME="$(echo $LIBRARY_FILENAME | sed -e 's/^libboost_//' -e 's/\.a$//')"
      OBJ_FOLDER_NAME="obj_$LIBRARY_NAME"
      OBJ_FOLDER_PATH="$PLATFORM_ARCHITECTURE_COMBO_BUILD_PATH/$OBJ_FOLDER_NAME"

      echo "  Decomposing $LIBRARY_FILENAME ..."

      # Must have separate obj folders for each library, because separate
      # libraries may contain object files with the same name.
      
      # In older versions of this script we sometimes built fat libraries, i.e. libraries
      # that contained multiple architectures. If that happened we had to use "lipo -thin"
      # for each architecture to extract the architecture-specific part from the fat
      # library, before we could use "ar" to extract the individual object files. This
      # is no longer necessary because we always build only one architecture.

      mkdir -p "$OBJ_FOLDER_PATH"
      cp "$LIBRARY_PATH" "$PLATFORM_ARCHITECTURE_COMBO_BUILD_PATH"
      (cd "$OBJ_FOLDER_PATH"; "$AR" -x "../$LIBRARY_FILENAME");
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
    echo "Linking all libraries for $PLATFORM_ARCHITECTURE_COMBO_NAME into a single library => $UNIVERSAL_LIBRARY_NAME"
    rm -f "$UNIVERSAL_LIBRARY_PATH"
    (cd "$PLATFORM_ARCHITECTURE_COMBO_BUILD_PATH"; "$LIBTOOL" -static -o "$UNIVERSAL_LIBRARY_PATH" obj_*/*.o;)

    buildFramework "$FRAMEWORK_BUNDLE_PATH" "$UNIVERSAL_LIBRARY_PATH"
  done
}

buildXcFramework()
{
  echo "Building XCFramework..."

  FRAMEWORK_ARGS=
  for FRAMEWORK_BUNDLE_PATH in $BUILD_DIR/*/$FRAMEWORK_BUNDLE_NAME; do
    FRAMEWORK_ARGS="$FRAMEWORK_ARGS -framework $FRAMEWORK_BUNDLE_PATH"
  done

  xcodebuild -create-xcframework \
             $FRAMEWORK_ARGS \
             -output "$BUILD_DIR/$XCFRAMEWORK_BUNDLE_NAME"
}

scrunchAllLibsTogetherInOneLibPerPlatformPerArchitecture
buildXcFramework

printf "$BUILD_PLATFORMS" > $BUILD_DIR/frameworks.built.platforms
printf "$LIBS_TO_BUILD" > $BUILD_DIR/frameworks.built.libs

#rm -rf "$BUILD_DIR/boost"

popd
