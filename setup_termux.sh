#!/bin/bash
# ─────────────────────────────────────────────────────────────
#  Meow Mod Menu — Termux Setup Script
#  Run this once inside Termux to push your project to GitHub.
# ─────────────────────────────────────────────────────────────

set -e  # Stop on any error

echo ""
echo "======================================"
echo "  Meow Mod Menu — GitHub Push Setup"
echo "======================================"
echo ""

# ── Step 1: Install git if not already installed ──
echo "[1/5] Installing git..."
pkg install -y git

# ── Step 2: Configure git identity ──
echo "[2/5] Configuring git identity..."
git config --global user.name  "Coloradodevly"
git config --global user.email "jjtjfud777@gmail.com"
echo "      Name:  Coloradodevly"
echo "      Email: jjtjfud777@gmail.com"

# ── Step 3: Unzip the project (if not already done) ──
echo "[3/5] Looking for imgui-android-mod-menu.zip..."

ZIP_PATH=""

# Check common download locations
for DIR in ~/storage/downloads ~/storage/shared/Download ~/Downloads; do
    if [ -f "$DIR/imgui-android-mod-menu.zip" ]; then
        ZIP_PATH="$DIR/imgui-android-mod-menu.zip"
        echo "      Found zip at: $ZIP_PATH"
        break
    fi
done

if [ -z "$ZIP_PATH" ]; then
    echo ""
    echo "  ⚠️  Could not find imgui-android-mod-menu.zip automatically."
    echo "  Please enter the full path to the zip file:"
    read -r ZIP_PATH
fi

echo "      Extracting..."
cd ~
unzip -o "$ZIP_PATH" -d ~
cd ~/imgui-android-mod-menu

# ── Step 4: Init git repo ──
echo "[4/5] Initialising git repository..."
git init
git add .
git commit -m "Initial commit — Meow mod menu"
git branch -M main
git remote add origin https://github.com/Coloradodevly/Meow.git

# ── Step 5: Push to GitHub ──
echo "[5/5] Pushing to GitHub..."
echo ""
echo "  You will be asked for your GitHub username and a Personal Access Token."
echo "  (GitHub no longer accepts passwords — use a PAT instead.)"
echo "  Create one at: https://github.com/settings/tokens"
echo "  Scopes needed: repo (full)"
echo ""
git push -u origin main

echo ""
echo "======================================"
echo "  Done! Your code is now on GitHub."
echo "  Repo: https://github.com/Coloradodevly/Meow"
echo ""
echo "  GitHub Actions will start building automatically."
echo "  Check the build at:"
echo "  https://github.com/Coloradodevly/Meow/actions"
echo ""
echo "  Download your .so from the Actions tab"
echo "  once the build goes green (~2-3 minutes)."
echo "======================================"
