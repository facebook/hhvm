<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function foo():this::T { throw new Exception("E"); }
}
