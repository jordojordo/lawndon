#!/bin/bash

set -e

PROJECT_NAME="lawndon-pi"
TARGET_ARCHIVE="$PROJECT_NAME.tar.gz"
PI_USER="pi"
PI_HOST="192.168.12.1"
PI_PATH="/home/pi"

echo "Starting the build and packaging process..."

echo "Cleaning up old builds..."
rm -rf $PROJECT_NAME $TARGET_ARCHIVE

echo "Building UI..."
pnpm install:ui
pnpm build:ui

echo "Building server..."
pnpm install:server
pnpm build:server

echo "Preparing the package directory..."
mkdir $PROJECT_NAME
cp -r server/dist $PROJECT_NAME/server
cp -r ui/dist $PROJECT_NAME/ui
cp -r config $PROJECT_NAME/
cp package.json $PROJECT_NAME/

echo "Creating the tar.gz archive..."
tar -czf $TARGET_ARCHIVE $PROJECT_NAME

rm -rf $PROJECT_NAME

echo "Build and transfer complete"
echo "To run the app on the Raspberry Pi:"
echo "1. SSH into your Raspberry Pi: ssh $PI_USER@$PI_HOST"
echo "2. Unpack the archive: tar -xzf $TARGET_ARCHIVE"
echo "3. Navigate to the project: cd $PROJECT_NAME"
echo "4. Start the backend server: node server/server.js"
