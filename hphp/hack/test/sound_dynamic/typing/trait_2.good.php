<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// No checks are performed upon encountering require extends

class C implements dynamic {}

class D {}

trait T1 implements dynamic {
  require extends C;
}

trait T2 implements dynamic {
  require extends D;
}

trait T3 {
  require extends C;
}
