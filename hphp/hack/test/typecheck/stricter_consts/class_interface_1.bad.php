<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class C {
  const int X = 1;
}

interface I {
  const int X = 2;
}

class D extends C implements I {}
