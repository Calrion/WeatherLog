#!/bin/sh
# update_version.sh
# WeatherLog
# Created by Greg Waterhouse, 2014-01-09.
#
#
# (c) Copyright 2014 Greg Waterhouse.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This script adds the Git commit hash to the Info.plist file, and sets
# CFBundleVersion to a Git-based build number. This happens automatically,
# so each build should be uniquely identifiable.
#
# You still need to manually set CFBundleShortVersionString!
#
# NOTE: This script will cancel a release build on a dirty repository.
#       Release builds MUST be made from a clean repository!

GIT=`xcrun -find git`
PBUDDY=`xcrun -find PlistBuddy 2> /dev/null`
if [ $? -ne 0 ]; then
	if [ -f "/usr/libexec/PlistBuddy" ]; then
		PBUDDY=/usr/libexec/PlistBuddy
	else
		echo "error: PlistBuddy not found."
	fi
fi

VERSION_NUMBER=`$PBUDDY -c "Print CFBundleShortVersionString" "${TARGET_BUILD_DIR}/${INFOPLIST_PATH}" 2> /dev/null`
BUILD_NUMBER=`$GIT rev-list --all HEAD | wc -l | egrep -o '[0-9]+' 2> /dev/null`
REPOID=`$GIT log -1 --format=format:%H 2> /dev/null`
REPO_DIRTY=0
$GIT update-index --refresh &> /dev/null
if [ $? -ne 0 ]; then
  REPO_DIRTY=1
  BUILD_NUMBER=${BUILD_NUMBER}m
fi

# Display build version data
echo "Building $PRODUCT_NAME v$VERSION_NUMBER ($BUILD_NUMBER)."
echo "Repository identifier is \"$REPOID\"."
if [ $REPO_DIRTY -eq 1 ]; then
	if [ "$CONFIGURATION" == "Release" ]; then
		echo "error: Working folder has uncommited changes. Commit, stash, or discard all changes before making a release."
		exit 1
	else
		echo "warning: Working folder has uncommitted changes."
	fi
fi

# Update Info.plist file
$PBUDDY -c "Set :CFBundleVersion ${BUILD_NUMBER}" "${TARGET_BUILD_DIR}/${INFOPLIST_PATH}" &> /dev/null
$PBUDDY -c "Set :CNVersionRepositoryIdentifier ${REPOID}" "${TARGET_BUILD_DIR}/${INFOPLIST_PATH}" &> /dev/null
if [ $? -ne 0 ]; then
	$PBUDDY -c "Add :CNVersionRepositoryIdentifier string ${REPOID}" "${TARGET_BUILD_DIR}/${INFOPLIST_PATH}" &> /dev/null
fi
