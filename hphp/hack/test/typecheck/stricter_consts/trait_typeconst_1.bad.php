<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Error: a child attempting to override a parent’s constant with a constant
// from a trait will fatal

class C {
  const type Ty = int;
}

trait T {
  const type Ty = string;
}

class D extends C {
  use T;
}
