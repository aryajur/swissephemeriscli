#!/usr/bin/env bash
set -euo pipefail

# Config
SWE_VERSION="2.10.03"   # change if needed
# Use GitHub releases instead of FTP/HTTP
SWE_URL_GITHUB="https://github.com/aloistr/swisseph/archive/refs/tags/v${SWE_VERSION}.tar.gz"
SWE_URL_HTTP="https://www.astro.com/ftp/swisseph/swe_unix_src_${SWE_VERSION}.tar.gz"
PREFIX="$(pwd)"
SWE_DIR="${PREFIX}/swisseph-src"
BUILD_DIR="${PREFIX}/build"

echo "==> Preparing directories"
mkdir -p "$BUILD_DIR" "$SWE_DIR"

# If sources missing, fetch or use local tarball
if [ ! -f "$SWE_DIR/swephexp.h" ] && [ ! -f "$SWE_DIR/src/swephexp.h" ]; then
  if ! tar -tzf "$BUILD_DIR/swe.tar.gz" >/dev/null 2>&1; then
    echo "==> Swiss Ephemeris tarball missing; attempting download"
    echo "    trying GitHub: $SWE_URL_GITHUB"
    (curl -L "$SWE_URL_GITHUB" -o "$BUILD_DIR/swe.tar.gz" || wget -O "$BUILD_DIR/swe.tar.gz" "$SWE_URL_GITHUB") || true
    if ! tar -tzf "$BUILD_DIR/swe.tar.gz" >/dev/null 2>&1; then
      echo "    GitHub failed; trying HTTP: $SWE_URL_HTTP"
      (curl -L "$SWE_URL_HTTP" -o "$BUILD_DIR/swe.tar.gz" || wget -O "$BUILD_DIR/swe.tar.gz" "$SWE_URL_HTTP") || true
    fi
  fi
  if ! tar -tzf "$BUILD_DIR/swe.tar.gz" >/dev/null 2>&1; then
    echo "!! Could not fetch a valid tar.gz."
    echo "   Place the official tarball at: $BUILD_DIR/swe.tar.gz and re-run."
    exit 1
  fi
  echo "==> Extracting Swiss Ephemeris"
  rm -rf "$SWE_DIR"/*
  tar -xzf "$BUILD_DIR/swe.tar.gz" -C "$SWE_DIR" --strip-components=1
fi

echo "==> Building Swiss Ephemeris"
# Handle both GitHub structure (root dir) and traditional structure (src/ subdir)
if [ -f "$SWE_DIR/Makefile" ]; then
  # GitHub structure - files in root directory
  make -C "$SWE_DIR" clean || true
  make -C "$SWE_DIR" libswe.a
elif [ -f "$SWE_DIR/src/makefile.gnu" ]; then
  # Traditional structure - files in src/ subdirectory
  make -C "$SWE_DIR/src" -f makefile.gnu clean || true
  make -C "$SWE_DIR/src" -f makefile.gnu libswe.a
else
  echo "!! No suitable makefile found in Swiss Ephemeris source"
  exit 1
fi

echo "==> Building Lua module"
make clean || true
# Forward LUA_DIR and LUA_PKG if provided in environment
make LUA_DIR="${LUA_DIR:-}" LUA_PKG="${LUA_PKG:-}"

echo "==> Done. Module built as swisseph.so"
