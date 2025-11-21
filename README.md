# KeyVal

A key-value database implementation in C++.

## Prerequisites

- C++ compiler with C++20 support
- Meson build system
- Ninja
- clang-format, clang-tidy, cppcheck

## Setup

```bash
git clone <repository-url>
cd KeyVal
./hooks/install.sh  # Install git hooks (optional)
meson setup dist
```

## Build & Run

```bash
./build.sh   # Format, analyze, build, and test
./test.sh    # Run tests only
./dev.sh     # Build and run the app
```

## Project Structure

```
KeyVal/
├── src/              # Source code
│   ├── models/       # BufferPool, DiskManager, Block
│   └── types/        # Constants and type definitions
├── tests/            # Unit tests
├── hooks/            # Git hooks
├── build.sh          # Build and analyze
├── test.sh           # Run tests
└── dev.sh            # Build and run
```

## Development

**Format code:**
```bash
clang-format -i src/**/*.cpp src/**/*.hpp
```

**Run tests:**
```bash
meson test -C dist
```

**Rebuild:**
```bash
rm -rf dist && meson setup dist
```

## License

See [LICENSE](LICENSE).