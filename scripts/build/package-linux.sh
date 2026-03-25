#!/bin/bash
# scripts/build/package-linux.sh
# Package script for Linux (creates tarball, deb, rpm, and AppImage)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_TYPE="${1:-release}"
PRESET="linux-clang-${BUILD_TYPE}"
BUILD_DIR="build/${PRESET}"
DIST_DIR="dist/linux"

cd "$PROJECT_ROOT"

# Build if needed
if [ ! -d "${BUILD_DIR}" ]; then
    echo "=== Building first ==="
    "${SCRIPT_DIR}/build-linux.sh" "${BUILD_TYPE}"
fi

echo "=== Creating output directory ==="
mkdir -p "${DIST_DIR}"

# Generate icon if needed
generate_icon() {
    local ICON_SIZES=("16" "22" "24" "32" "48" "64" "128" "256" "512")
    local ICON_DIR="${PROJECT_ROOT}/build/icons"
    
    if command -v convert &>/dev/null || command -v magick &>/dev/null; then
        mkdir -p "${ICON_DIR}"
        
        MAGICK_CMD="magick"
        if ! command -v magick &>/dev/null; then
            MAGICK_CMD="convert"
        fi
        
        for size in "${ICON_SIZES[@]}"; do
            echo "Generating ${size}x${size} icon..."
            $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" \
                -background none \
                -resize "${size}x${size}" \
                "${ICON_DIR}/markdown-viewer-${size}.png"
        done
        
        # Create PNG for AppImage
        $MAGICK_CMD "${PROJECT_ROOT}/resources/icon.png" \
            -background none \
            -resize 256x256 \
            "${PROJECT_ROOT}/resources/icon-256.png"
    else
        echo "Warning: ImageMagick not found, skipping icon generation"
    fi
}

# Package with CPack
echo "=== Creating CPack packages ==="
cd "${BUILD_DIR}"
cpack -G "TGZ;DEB;RPM"
cd "$PROJECT_ROOT"

# Create AppImage
echo "=== Creating AppImage ==="
generate_icon

APPDIR="${PROJECT_ROOT}/AppDir"
mkdir -p "${APPDIR}/usr/bin"
mkdir -p "${APPDIR}/usr/share/applications"
mkdir -p "${APPDIR}/usr/share/icons/hicolor"

# Copy binary
cp "${BUILD_DIR}/bin/markdown_viewer" "${APPDIR}/usr/bin/"

# Copy .desktop file
cp "${PROJECT_ROOT}/resources/markdown-viewer.desktop" "${APPDIR}/usr/share/applications/"

# Copy icons to AppDir
if [ -d "${PROJECT_ROOT}/build/icons" ]; then
    for size in 16 22 24 32 48 64 128 256 512; do
        icon_file="${PROJECT_ROOT}/build/icons/markdown-viewer-${size}.png"
        if [ -f "$icon_file" ]; then
            mkdir -p "${APPDIR}/usr/share/icons/hicolor/${size}x${size}/apps"
            cp "$icon_file" "${APPDIR}/usr/share/icons/hicolor/${size}x${size}/apps/markdown-viewer.png"
        fi
    done
fi

# Copy 256px icon for linuxdeploy
if command -v magick >/dev/null 2>&1; then
    magick "${PROJECT_ROOT}/resources/icon.png" -resize 256x256^ -gravity center -extent 256x256 "${APPDIR}/markdown-viewer.png"
elif command -v convert >/dev/null 2>&1; then
    convert "${PROJECT_ROOT}/resources/icon.png" -resize 256x256^ -gravity center -extent 256x256 "${APPDIR}/markdown-viewer.png"
else
    cp "${PROJECT_ROOT}/resources/icon.png" "${APPDIR}/markdown-viewer.png"
fi

# Download linuxdeploy if not present
LINUXDEPLOY="${PROJECT_ROOT}/tools/local/linuxdeploy-x86_64.AppImage"
if [ ! -f "$LINUXDEPLOY" ]; then
    echo "Downloading linuxdeploy..."
    mkdir -p "${PROJECT_ROOT}/tools/local"
    if ! command -v curl >/dev/null 2>&1; then
        echo "Error: curl is required to download linuxdeploy securely." >&2
        exit 1
    fi

    LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    FINAL_URL="$(curl --fail --silent --show-error --location --proto '=https' --proto-redir '=https' --tlsv1.2 --retry 3 --output "$LINUXDEPLOY" --write-out '%{url_effective}' "$LINUXDEPLOY_URL")"
    case "$FINAL_URL" in
        https://github.com/*|https://objects.githubusercontent.com/*|https://release-assets.githubusercontent.com/*)
            ;;
        *)
            echo "Error: Unexpected linuxdeploy redirect target: $FINAL_URL" >&2
            rm -f "$LINUXDEPLOY"
            exit 1
            ;;
    esac
    chmod +x "$LINUXDEPLOY"
fi

# Build AppImage
export VERSION="${VERSION:-1.0.0}"
export APPIMAGE_EXTRACT_AND_RUN=1
"$LINUXDEPLOY" \
    --appdir "${APPDIR}" \
    -e "${APPDIR}/usr/bin/markdown_viewer" \
    -d "${PROJECT_ROOT}/resources/markdown-viewer.desktop" \
    -i "${APPDIR}/markdown-viewer.png" \
    --output appimage

# Move AppImage to dist
mv -v Markdown_Viewer*.AppImage "${DIST_DIR}/" 2>/dev/null || mv -v markdown-viewer*.AppImage "${DIST_DIR}/" 2>/dev/null || true

# Move other packages to dist
mv -v "${BUILD_DIR}"/*.tar.gz "${DIST_DIR}/" 2>/dev/null || true
mv -v "${BUILD_DIR}"/*.deb "${DIST_DIR}/" 2>/dev/null || true
mv -v "${BUILD_DIR}"/*.rpm "${DIST_DIR}/" 2>/dev/null || true

echo "=== Packaging complete ==="
echo "Packages in ${DIST_DIR}:"
ls -la "${DIST_DIR}/"
