<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('enum_class')>>

enum class E0 : mixed {
  A<int>(42);
  B<string>('zuck');
}

enum class F0 : mixed {
  C<int>(0);
  D<string>('foo');
}


// E1 --> E0 is ok
enum class E1 : mixed extends E0 {
  X<int>(1664);
}

// E2 --> E0
// |----> F0
// is ok (V shape)
enum class E2 : mixed extends E0, F0 {
  X<int>(1664);
}
