#!/bin/bash
echo "=== Formatting Code ==="
find . -name "*.cpp" -o -name "*.hpp" -print0 | xargs -0 -r clang-format -i

echo "=== Running clang-tidy (recursively, using dist) ==="
if [ ! -f dist/compile_commands.json ]; then
  echo "dist/compile_commands.json not found; running meson setup dist..."
  meson setup dist >/dev/null 2>&1 || true
fi

find . -name "*.cpp" -print0 | xargs -0 -r -I{} clang-tidy -p dist {} -- -Iinclude -Isrc -std=c++20 || true

echo "=== Running cppcheck ==="
cppcheck --enable=all --std=c++20 -Iinclude -Isrc --suppress=missingIncludeSystem src/

echo "=== Building ==="
meson compile -C dist

echo "=== Testing ==="
meson test -C dist
