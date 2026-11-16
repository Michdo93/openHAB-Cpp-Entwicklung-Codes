#!/bin/sh
# Übersetzt alle Beispiele (beim ersten Mal alles) und startet eines –
# mit den Zugangsdaten aus .env.
#   ./starte.sh kapitel10_erste_schritte/01_verbinden.cpp [Argumente]
set -e
cd "$(dirname "$0")"
set -a
[ -f .env ] && . ./.env
set +a
if [ ! -f build/CMakeCache.txt ]; then
  cmake -S . -B build >/dev/null
fi
cmake --build build -j2 >/dev/null
quelle=$1; shift
programm=build/$(dirname "$quelle")/$(basename "$quelle" .cpp)
exec "$programm" "$@"
