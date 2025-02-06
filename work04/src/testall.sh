#!/bin/bash

DIRECTORY="../../work01/src/mppl"
EXEC_FILE="./cr"

# アルファベット順でファイルを処理
for FILE in $(ls "$DIRECTORY" | sort); do
  FULL_PATH="$DIRECTORY/$FILE"
  if [ -f "$FULL_PATH" ]; then
    echo "Processing file: $FILE"
    # 必要な処理をここに書く
    $EXEC_FILE $FULL_PATH
    echo ""
  fi
done

DIRECTORY="../../work02/src/mppl"

# アルファベット順でファイルを処理
for FILE in $(ls "$DIRECTORY" | sort); do
  FULL_PATH="$DIRECTORY/$FILE"
  if [ -f "$FULL_PATH" ]; then
    echo "Processing file: $FILE"
    # 必要な処理をここに書く
    $EXEC_FILE $FULL_PATH
    echo ""
  fi
done

DIRECTORY="../mppl"

# アルファベット順でファイルを処理
for FILE in $(ls "$DIRECTORY" | sort); do
  FULL_PATH="$DIRECTORY/$FILE"
  if [ -f "$FULL_PATH" ]; then
    echo "Processing file: $FILE"
    # 必要な処理をここに書く
    $EXEC_FILE $FULL_PATH
    echo ""
  fi
done
