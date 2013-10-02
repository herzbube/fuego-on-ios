# Modified from the script at
# https://github.com/justinweiss/fuego-framework/blob/master/build_framework.sh
# which in turn is a modified version of the script at
# http://codefriend.blogspot.com/2011/09/creating-ios-framework-with-xcode4.html


# Sets the target folders and the final framework product.
FRAMEWORK_NAME="fuego-on-ios"
FRAMEWORK_VERSION="A"

# Install dir will be the final output to the framework.
# The following line create it in the root folder of the current project.
FRAMEWORK_FOLDER="$(pwd)/ios/framework/${FRAMEWORK_NAME}.framework"


IPHONEOS_SDKPREFIX="iphoneos"
IPHONE_SIMULATOR_SDKPREFIX="iphonesimulator"
CONFIGURATION="Release"

BUILD_FOLDER="$(pwd)/build"
IPHONEOS_BUILD_FOLDER="${BUILD_FOLDER}/$CONFIGURATION-$IPHONEOS_SDKPREFIX"
IPHONE_SIMULATOR_BUILD_FOLDER="${BUILD_FOLDER}/$CONFIGURATION-$IPHONE_SIMULATOR_SDKPREFIX"
BUILD_LOGFILE_NAME="build.log"


# Building both architectures.
>"$BUILD_LOGFILE_NAME"
if test $? -ne 0; then
  echo "Aborting, cannot write to build log file $BUILD_LOGFILE_NAME"
  exit 1
fi
xcodebuild -configuration "$CONFIGURATION" -target "${FRAMEWORK_NAME}" -sdk "$IPHONEOS_SDKPREFIX" 2>&1 | tee -a "$BUILD_LOGFILE_NAME"
xcodebuild -configuration "$CONFIGURATION" -target "${FRAMEWORK_NAME}" -sdk "$IPHONE_SIMULATOR_SDKPREFIX" 2>&1 | tee -a "$BUILD_LOGFILE_NAME"

# Cleaning the oldest.
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

# Copies the headers and resources files to the final product folder.
cp -R "${IPHONEOS_BUILD_FOLDER}/Headers/" "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}/Headers/"
cp -R "${IPHONEOS_BUILD_FOLDER}/" "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}/Resources/"

# Removes the binary and header from the resources folder.
rm -r "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}/Resources/Headers" "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}/Resources/lib${FRAMEWORK_NAME}.a"

# Uses the Lipo Tool to merge both binary files (i386 + armv7/armv7s) into one Universal final product.
lipo -create "${IPHONEOS_BUILD_FOLDER}/lib${FRAMEWORK_NAME}.a" "${IPHONE_SIMULATOR_BUILD_FOLDER}/lib${FRAMEWORK_NAME}.a" -output "${FRAMEWORK_FOLDER}/Versions/${FRAMEWORK_VERSION}/${FRAMEWORK_NAME}"

echo "Build finished"
echo "The framework is located here: $FRAMEWORK_FOLDER"
echo "Enjoy."
