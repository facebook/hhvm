<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  const dict<int, string> C1 = dict[2 => "folly"];
  const dict<int, dict<int, string>> C2 = dict[2 => dict[4 => "folly"]];
  abstract const int C3;
}
