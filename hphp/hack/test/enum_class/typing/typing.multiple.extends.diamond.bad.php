<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('enum_class')>>

// Testing diamond shapes
enum class D0 : mixed {
  A<int>(42);
  B<string>('zuck');
}

enum class D1 : mixed extends D0 {
  C<int>(1664);
}

enum class D2 : mixed extends D0 {
  D<string>('');
}

// D3 --> D1 ---> D0
// |----> D2------^
// diamons are never ok, and detected at redeclaring constants from the
// root (D0)
enum class D3: mixed extends D1, D2 {
  E<int>(0);
}
