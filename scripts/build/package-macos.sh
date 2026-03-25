#!/bin/bash
# scripts/build/package-macos.sh
# Package script for macOS (creates DMG and ZIP)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_TYPE="${1:-release}"
PRESET="macos-clang-${BUILD_TYPE}"
BUILD_DIR="build/${PRESET}"
DIST_DIR="dist/macos"

cd "$PROJECT_ROOT"

# Build if needed
if [ ! -d "${BUILD_DIR}" ]; then
    echo "=== Building first ==="
    "${SCRIPT_DIR}/build-macos.sh" "${BUILD_TYPE}"
fi

echo "=== Creating output directory ==="
mkdir -p "${DIST_DIR}"

# Generate icon if needed
generate_icns() {
    if command -v magick &>/dev/null || command -v convert &>/dev/null; then
        MAGICK_CMD="magick"
        if ! command -v magick &>/dev/null; then
            MAGICK_CMD="convert"
        fi
        
        ICONSET="${PROJECT_ROOT}/build/icon.iconset"
        mkdir -p "$ICONSET"
        
        # Generate all required sizes for icns
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 16x16 "${ICONSET}/icon_16x16.png"
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 32x32 "${ICONSET}/icon_16x16@2x.png"
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 32x32 "${ICONSET}/icon_32x32.png"
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 64x64 "${ICONSET}/icon_32x32@2x.png"
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 128x128 "${ICONSET}/icon_128x128.png"
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 256x256 "${ICONSET}/icon_128x128@2x.png"
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 256x256 "${ICONSET}/icon_256x256.png"
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 512x512 "${ICONSET}/icon_256x256@2x.png"
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 512x512 "${ICONSET}/icon_512x512.png"
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" -resize 1024x1024 "${ICONSET}/icon_512x512@2x.png"
        
        # Create icns
        iconutil -c icns "$ICONSET" -o "${PROJECT_ROOT}/resources/icon.icns" 2>/dev/null || true
    fi
}

# Generate icons
echo "=== Generating icons ==="
generate_icns

# Package with CPack
echo "=== Creating CPack packages ==="
cd "${BUILD_DIR}"
cpack -G "DragNDrop;TGZ"
cd "$PROJECT_ROOT"

# Move packages to dist
mv -v "${BUILD_DIR}"/*.dmg "${DIST_DIR}/" 2>/dev/null || true
mv -v "${BUILD_DIR}"/*.tar.gz "${DIST_DIR}/" 2>/dev/null || true

echo "=== Packaging complete ==="
echo "Packages in ${DIST_DIR}:"
ls -la "${DIST_DIR}/"
