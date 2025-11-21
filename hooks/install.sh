#!/bin/sh

echo "Installing Git hooks..."

# Get the git hooks directory
GIT_HOOKS_DIR=".git/hooks"

# Check if .git directory exists
if [ ! -d ".git" ]; then
    echo "❌ Error: Not a git repository. Run this from the project root."
    exit 1
fi

# Copy pre-push hook
cp hooks/pre-push "$GIT_HOOKS_DIR/pre-push"

# Make it executable
chmod +x "$GIT_HOOKS_DIR/pre-push"

echo "✅ Git hooks installed successfully!"
echo "The pre-push hook will now run formatting checks before every push."