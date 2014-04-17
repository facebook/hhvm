<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

namespace NSTest;

function f<T>(T $x, bool $b): int {
  echo "namespaces are fun\n";
  return 1;
}

f(1, true);
