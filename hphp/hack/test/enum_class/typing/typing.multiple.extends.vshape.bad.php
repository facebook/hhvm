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

enum class F1 : mixed {
   int B = 1664;
}

// V1 --> E0
// |----> F1
// is not ok (V shape), F1 as same constant name than E0 but different type
enum class V1 : mixed extends E0, F1 {
   int X = 1664;
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

enum class F3 : mixed {
   string C = '';
}

// V3 --> E0
// |----> F3
// is not ok (V shape), V3 has same constant as one in parent
enum class V3 : mixed extends E0, F3 {
   string C = '';
}

// V4 --> E0
// |----> F3
// is not ok (V shape), V4 has same constant with different type
enum class V4 : mixed extends E0, F3 {
   int C = 42;
}
// V5 --> E0
// |----> F3
// is not ok (V shape), V4 has same constant with different value
enum class V5 : mixed extends E0, F3 {
   string C = 'foo';
}
