<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface IC {
  const int X = 1;
}

class C implements IC {}

interface ID {
  const int X = 2;
}

class D extends C implements ID {}
