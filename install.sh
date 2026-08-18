#!/usr/bin/env bash
set -e

echo "========================================="
echo "   🚀 RamaCraft: Linux Installer        "
echo "  Voxel Survival in Rendezvous with Rama "
echo "========================================="

DEB_URL="https://github.com/Augustalex/ramacraft/releases/latest/download/ramacraft-ubuntu-debian-amd64.deb"
TAR_URL="https://github.com/Augustalex/ramacraft/releases/latest/download/ramacraft-linux-x64.tar.gz"

# Check if Debian / Ubuntu / Kubuntu system
if command -v apt-get >/dev/null 2>&1; then
    echo "📦 Detected Debian / Ubuntu / Kubuntu system."
    TEMP_DEB=$(mktemp /tmp/ramacraft-XXXXXX.deb)
    echo "⬇️  Downloading RamaCraft .deb package..."

    if command -v curl >/dev/null 2>&1; then
        curl -sSL "$DEB_URL" -o "$TEMP_DEB"
    elif command -v wget >/dev/null 2>&1; then
        wget -q "$DEB_URL" -O "$TEMP_DEB"
    else
        echo "❌ Error: Neither curl nor wget was found. Please install curl or wget first."
        exit 1
    fi

    echo "⚙️  Installing RamaCraft and dependencies..."
    sudo apt-get update -qq
    sudo apt-get install -y "$TEMP_DEB"
    rm -f "$TEMP_DEB"

    echo ""
    echo "========================================="
    echo "   ✅ RamaCraft Installed Successfully! "
    echo "========================================="
    echo "🎮 Launch from your Application Menu (Games) or type: ramacraft"
    echo ""
    
    if [ -t 0 ]; then
        read -p "Would you like to launch RamaCraft now? [Y/n] " -n 1 -r
        echo ""
        if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
            ramacraft &
        fi
    else
        echo "Run 'ramacraft' in your terminal to play!"
    fi
else
    # Standalone portable installation for Fedora / Arch / other Linux distros
    INSTALL_DIR="$HOME/.local/share/ramacraft"
    BIN_DIR="$HOME/.local/bin"

    echo "📦 Installing standalone portable RamaCraft to $INSTALL_DIR..."
    mkdir -p "$INSTALL_DIR" "$BIN_DIR"
    cd "$INSTALL_DIR"

    if command -v curl >/dev/null 2>&1; then
        curl -sSL "$TAR_URL" | tar -xz
    elif command -v wget >/dev/null 2>&1; then
        wget -qO- "$TAR_URL" | tar -xz
    fi

    chmod +x "$INSTALL_DIR/ramacraft"
    if [ -f "$INSTALL_DIR/run.sh" ]; then
        chmod +x "$INSTALL_DIR/run.sh"
    fi

    ln -sf "$INSTALL_DIR/ramacraft" "$BIN_DIR/ramacraft"

    echo ""
    echo "========================================="
    echo "   ✅ RamaCraft Installed Successfully! "
    echo "========================================="
    echo "🎮 Launching RamaCraft..."
    "$INSTALL_DIR/ramacraft" &
fi
