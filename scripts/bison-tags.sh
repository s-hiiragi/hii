#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
TRACELOG="$SCRIPT_DIR/../out/parser.output"
TAGS="$SCRIPT_DIR/../tags"

egrep '^State [0-9]+' "$TRACELOG" \
  | cut -d ' ' -f 2 \
  | while read l
do
#ARRAY	cvalue.h	/^        ARRAY,$/;"	e	enum:cvalue::type_t
    echo -e "bison_tracelog_state$l\t$TRACELOG\t/^State $l/"
done >> "$TAGS"

sort -o "$TAGS" "$TAGS"

