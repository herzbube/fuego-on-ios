# Modified from the script at
# https://github.com/justinweiss/fuego-framework/blob/master/build_framework.sh
# which in turn is a modified version of the script at
# http://codefriend.blogspot.com/2011/09/creating-ios-framework-with-xcode4.html

IPHONEOS_SDKPREFIX="iphoneos"
IPHONE_SIMULATOR_SDKPREFIX="iphonesimulator"

# Base SDK versions. Can be overridden by the caller. If left undefined, the
# latest SDK known to your Xcode will be used
: ${IPHONEOS_BASESDK_VERSION:=`xcrun --sdk $IPHONEOS_SDKPREFIX --show-sdk-version`}
: ${IPHONE_SIMULATOR_BASESDK_VERSION:=`xcrun --sdk $IPHONE_SIMULATOR_SDKPREFIX --show-sdk-version`}

# Deployment target versions. Can be overridden by the caller.
: ${IPHONEOS_DEPLOYMENT_TARGET:=12.0}
: ${IPHONE_SIMULATOR_DEPLOYMENT_TARGET:=12.0}

# Sets the target folders and the final framework product.
FRAMEWORK_NAME="fuego-on-ios"
FRAMEWORK_VERSION="A"
FRAMEWORK_FOLDER_NAME="${FRAMEWORK_NAME}.framework"
XCFRAMEWORK_FOLDER="$(pwd)/ios/framework/${FRAMEWORK_NAME}.xcframework"

# Setup a few more variables
IPHONEOS_SDKNAME="${IPHONEOS_SDKPREFIX}${IPHONEOS_BASESDK_VERSION}"
IPHONE_SIMULATOR_SDKNAME="${IPHONE_SIMULATOR_SDKPREFIX}${IPHONE_SIMULATOR_BASESDK_VERSION}"
CONFIGURATION="Release"
BUILD_FOLDER="$(pwd)/build"
IPHONEOS_BUILD_FOLDER="${BUILD_FOLDER}/$CONFIGURATION-$IPHONEOS_SDKPREFIX"
IPHONE_SIMULATOR_BUILD_FOLDER="${BUILD_FOLDER}/$CONFIGURATION-$IPHONE_SIMULATOR_SDKPREFIX"
IPHONEOS_FRAMEWORK_FOLDER="${IPHONEOS_BUILD_FOLDER}/${FRAMEWORK_FOLDER_NAME}"
IPHONE_SIMULATOR_FRAMEWORK_FOLDER="${IPHONE_SIMULATOR_BUILD_FOLDER}/${FRAMEWORK_FOLDER_NAME}"
XCCONFIG_FILE="${BUILD_FOLDER}/fuego-on-ios.xcconfig"

# Populate the .xcconfig file that overrides project settings
rm -f "$XCCONFIG_FILE"
mkdir -p "$BUILD_FOLDER"
echo "IPHONEOS_DEPLOYMENT_TARGET = $IPHONEOS_DEPLOYMENT_TARGET" >>"$XCCONFIG_FILE"
echo "IPHONE_SIMULATOR_BASESDK_VERSION = $IPHONE_SIMULATOR_BASESDK_VERSION" >>"$XCCONFIG_FILE"

# Building both platforms.
xcodebuild -xcconfig "$XCCONFIG_FILE" -configuration "$CONFIGURATION" -target "${FRAMEWORK_NAME}" -sdk "$IPHONEOS_SDKNAME"
if test $? -ne 0; then
  exit 1
fi
xcodebuild -xcconfig "$XCCONFIG_FILE" -configuration "$CONFIGURATION" -target "${FRAMEWORK_NAME}" -sdk "$IPHONE_SIMULATOR_SDKNAME"
if test $? -ne 0; then
  exit 1
fi

# Create the frameworks
for PLATFORM_BUILD_FOLDER in ${IPHONEOS_BUILD_FOLDER} ${IPHONE_SIMULATOR_BUILD_FOLDER}; do
  if test "${PLATFORM_BUILD_FOLDER}" = "${IPHONEOS_BUILD_FOLDER}"; then
    FRAMEWORK_FOLDER="${IPHONEOS_FRAMEWORK_FOLDER}"
  else
    FRAMEWORK_FOLDER="${IPHONE_SIMULATOR_FRAMEWORK_FOLDER}"
  fi

  # Cleaning the previous build if it exists
  if test -d "${FRAMEWORK_FOLDER}"; then
    rm -rf "${FRAMEWORK_FOLDER}"
  fi

  # Creates and renews the final product folder.
  mkdir -p "${FRAMEWORK_FOLDER}"
  mkdir -p "${FRAMEWORK_FOLDER}/Versions"
  mkdir -p "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}"
  mkdir -p "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}/Resources"
  mkdir -p "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}/Headers"

  # Creates the internal links.
  # It MUST uses relative path, otherwise will not work when the folder is copied/moved.
  ln -s "${FRAMEWORK_VERSION}" "${FRAMEWORK_FOLDER}/Versions/Current"
  ln -s "Versions/Current/Headers" "${FRAMEWORK_FOLDER}/Headers"
  ln -s "Versions/Current/Resources" "${FRAMEWORK_FOLDER}/Resources"
  ln -s "Versions/Current/${FRAMEWORK_NAME}" "${FRAMEWORK_FOLDER}/${FRAMEWORK_NAME}"

  # Copies the headers and binary files to the final product folder.
  cp -R "${PLATFORM_BUILD_FOLDER}/Headers/" "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}/Headers/"
  cp "${PLATFORM_BUILD_FOLDER}/lib${FRAMEWORK_NAME}.a" "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}/${FRAMEWORK_NAME}"
done

# Create the final XCFramework
if test -d "${XCFRAMEWORK_FOLDER}"; then
  rm -rf "${XCFRAMEWORK_FOLDER}"
fi
xcodebuild -create-xcframework -framework "${IPHONEOS_FRAMEWORK_FOLDER}" -framework "${IPHONE_SIMULATOR_FRAMEWORK_FOLDER}" -output "$XCFRAMEWORK_FOLDER"
if test $? -ne 0; then
  exit 1
fi

echo "Build finished"
echo "The framework is located here: $XCFRAMEWORK_FOLDER"
echo "Enjoy."
