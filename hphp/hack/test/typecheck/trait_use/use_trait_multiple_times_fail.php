<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T {}

class C {
  // using the same trait multiple times in a class is forbidden
  use T;
  use T;
  use T;
}
