#!/usr/bin/env bash
# Собирает проект вне исходного дерева в build/ и запускает qt-paint.
#   service_scripts/run.sh            — собрать и открыть окно
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
mkdir -p "$root/build"
cd "$root/build"
qmake "$root/qt-paint.pro" >/dev/null
make -j4 2>&1 | grep -E "warning|error" || true
test -x ./qt-paint
./qt-paint
