#!/bin/bash

DIRECTORY="../../mppl"
EXEC_FILE="./tc"

# アルファベット順でファイルを処理
for FILE in $(ls "$DIRECTORY" | sort); do
  FULL_PATH="$DIRECTORY/$FILE"
  if [ -f "$FULL_PATH" ]; then
    echo "Processing file: $FILE"
    # 必要な処理をここに書く
    $EXEC_FILE $FULL_PATH
  fi
done
