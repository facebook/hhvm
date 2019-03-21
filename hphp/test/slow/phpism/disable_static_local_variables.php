<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo() {
  static $x = 5;
}

foo();
echo "It works!\n";
