<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum class E0 : mixed {
   int A = 42;
   string B = 'zuck';
}

enum class F0 : mixed {
   int C = 0;
   string D = 'foo';
}


// E1 --> E0 is ok
enum class E1 : mixed extends E0 {
   int X = 1664;
}

// E2 --> E0
// |----> F0
// is ok (V shape)
enum class E2 : mixed extends E0, F0 {
   int X = 1664;
}

<<__EntryPoint>>
function main(): void {
  var_dump(E1::A === 42);
  var_dump(E2::A === 42);
  var_dump(E1::X === E2::X);
  var_dump(E2::D === 'foo');
}
