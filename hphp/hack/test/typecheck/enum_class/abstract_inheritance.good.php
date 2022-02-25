<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract enum class E : mixed {
  abstract int X;
}

enum class F : mixed extends E {
  int X = 42;
}

abstract enum class G : mixed extends F {
  abstract int Y;
}

enum class H : mixed extends G {
  int Y = 42;
}
