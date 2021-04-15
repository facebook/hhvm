<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Testing V shapes
enum class E0 : mixed {
   int A = 42;
   string B = 'zuck';
}

enum class F1 : mixed {
   int B = 1664;
}

// V1 --> E0
// |----> F1
// is not ok (V shape), F1 as same constant name than E0 but different type
enum class V1 : mixed extends E0, F1 {
   int X = 1664;
}

<<__EntryPoint>>
function main(): void {
  echo "Should not see this!\n";
}
