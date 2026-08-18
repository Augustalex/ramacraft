#!/usr/bin/env bash
set -e

echo "========================================="
echo "   🔄 Updating RamaCraft to Latest...   "
echo "========================================="

# Call the primary installer/updater script
curl -sSL https://raw.githubusercontent.com/Augustalex/ramacraft/main/install.sh | bash
