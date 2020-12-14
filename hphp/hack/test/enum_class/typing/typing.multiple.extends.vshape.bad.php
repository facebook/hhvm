<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('enum_class')>>

// Testing V shapes
enum class E0 : mixed {
  A<int>(42);
  B<string>('zuck');
}

enum class F0 : mixed {
  B<string>('zuck');
}

// V0 --> E0
// |----> F0
// is not ok (V shape), E0 and F0 have the same constants
enum class V0 : mixed extends E0, F0 {
  X<int>(1664);
}

enum class F1 : mixed {
  B<int>(1664);
}

// V1 --> E0
// |----> F1
// is not ok (V shape), F1 as same constant name than E0 but different type
enum class V1 : mixed extends E0, F1 {
  X<int>(1664);
}

enum class F2 : mixed {
  B<string>('');
}

// V2 --> E0
// |----> F2
// is not ok (V shape), F2 as same constant name than E0 but different value
enum class V2 : mixed extends E0, F2 {
  X<int>(1664);
}

enum class F3 : mixed {
  C<string>('');
}

// V3 --> E0
// |----> F3
// is not ok (V shape), V3 has same constant as one in parent
enum class V3 : mixed extends E0, F3 {
  C<string>('');
}

// V4 --> E0
// |----> F3
// is not ok (V shape), V4 has same constant with different type
enum class V4 : mixed extends E0, F3 {
  C<int>(42);
}
// V5 --> E0
// |----> F3
// is not ok (V shape), V4 has same constant with different value
enum class V5 : mixed extends E0, F3 {
  C<string>('foo');
}
