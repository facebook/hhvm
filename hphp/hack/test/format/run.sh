#! /usr/bin/env bash

set -e

HH_FORMAT=$1
ROOT=$2

$HH_FORMAT --root $ROOT $ROOT/simple.php > $ROOT/simple.php.out
diff $ROOT/simple.php.exp $ROOT/simple.php.out
