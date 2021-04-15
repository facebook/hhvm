<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Testing V shapes
enum class E0 : mixed {
   int A = 42;
   string B = 'zuck';
}

enum class F0 : mixed {
   string B = 'zuck';
}

// V0 --> E0
// |----> F0
// is not ok (V shape), E0 and F0 have the same constants
enum class V0 : mixed extends E0, F0 {
   int X = 1664;
}

<<__EntryPoint>>
function main(): void {
  echo "Should not see this!\n";
}
