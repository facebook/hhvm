<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// A trait that does not implement dynamic cannot be used by a class that
// implements dynamic

class C implements dynamic {
  use T;
}

trait T {}
