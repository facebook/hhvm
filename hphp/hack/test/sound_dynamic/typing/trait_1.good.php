<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// A trait that implements dynamic can be used both by classes that do
// and that do not implement dynamic

<<__SoundDynamicCallable>>
class C {
  use T;
}

class D {
  use T;
}

<<__SoundDynamicCallable>>
trait T {}
