<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract enum class E : mixed {
  abstract int X;
}

enum class F : mixed extends E {
}

abstract enum class G : mixed extends F {
  int X = 42;
  abstract int Y;
}

abstract enum class EE : mixed {
  abstract int X;
}

enum class FF : mixed extends EE {
  int X = 42;
}

abstract enum class GG : mixed extends FF {
  abstract int X;
}
