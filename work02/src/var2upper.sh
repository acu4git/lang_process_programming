#!/bin/bash

# 引数チェック
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <source_file>"
  exit 1
fi

# 対象のファイル
source_file="$1"

# バックアップを作成
backup_file="${source_file}.bak"
cp "$source_file" "$backup_file"

# 正規表現で変数名を変換
sed -E -i '
# errmsg_*** 形式の変数名をキャプチャして大文字スネークケースに変換
s/errmsg_([a-z0-9_]+)/errmsg_\U\1/g
s/(const char\* )errmsg_([A-Z0-9_]+)/\1\Uerrmsg_\2/g
' "$source_file"

echo "変換が完了しました。元のファイルは ${backup_file} として保存されています。"
