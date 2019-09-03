#!/bin/bash
cd "$(dirname "$0")"
cd bin
chmod +x ai_yang
./ai_yang "$1" "$2" "$3"
