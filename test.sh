#!/bin/bash

echo "=== Building ==="
meson compile -C dist

echo "=== Testing ==="
meson test -C dist
