#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
TAGS="$SCRIPT_DIR/../tags"

# for tracelog
TRACELOG="$SCRIPT_DIR/../out/parser.output"
egrep '^State [0-9]+' "$TRACELOG" \
  | cut -d ' ' -f 2 \
  | while read l
do
    echo -e "bison_output_state$l\t$TRACELOG\t/^State $l/"
done >> "$TAGS"

# for parser
PARSER_FILE="$SCRIPT_DIR/../parser.yy"
egrep '^[a-z_]+\s*:' "$PARSER_FILE" \
  | tr ':' ' '  \
  | awk '{print $1}' \
  | while read l
do
    echo -e "bison_parser_nterm_$l\t$PARSER_FILE\t/^$l\\s\\{0,\\}:/"
done >> "$TAGS"

sort -o "$TAGS" "$TAGS"

