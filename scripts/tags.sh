#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")

(cd "$SCRIPT_DIR/.." && ctags -R)
"$SCRIPT_DIR/bison-tags.sh"

