set -e

cd /Users/jguida941/final_project/cmake-build-debug
/Applications/CLion.app/Contents/bin/cmake/mac/aarch64/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
