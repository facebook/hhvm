<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait TA {
 const string ONCALL = "a";
}

trait TB {
  const string ONCALL = "b";
}

class A {
  use TA;
}

final class B extends A {
 use TB;
}
