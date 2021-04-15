<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Testing V shapes
enum class E0 : mixed {
   int A = 42;
   string B = 'zuck';
}

enum class F3 : mixed {
   string C = '';
}

// V3 --> E0
// |----> F3
// is not ok (V shape), V3 has same constant as one in parent
enum class V3 : mixed extends E0, F3 {
   string C = '';
}

<<__EntryPoint>>
function main(): void {
  echo "Should not see this!\n";
}
