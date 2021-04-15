<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Testing V shapes
enum class E0 : mixed {
   int A = 42;
   string B = 'zuck';
}

enum class F2 : mixed {
   string B = '';
}

// V2 --> E0
// |----> F2
// is not ok (V shape), F2 as same constant name than E0 but different value
enum class V2 : mixed extends E0, F2 {
   int X = 1664;
}

<<__EntryPoint>>
function main(): void {
  echo "Should not see this!\n";
}
