<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Testing diamond shapes
enum class D0 : mixed {
   int A = 42;
   string B = 'zuck';
}

enum class D1 : mixed extends D0 {
   int C = 1664;
}

enum class D2 : mixed extends D0 {
   string D = '';
}

// D3 --> D1 ---> D0
// |----> D2------^
// diamons are never ok, and detected at redeclaring constants from the
// root (D0)
enum class D3: mixed extends D1, D2 {
   int E = 0;
}
