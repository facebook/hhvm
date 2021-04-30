<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// No checks are performed upon encountering require extends

<<__SupportDynamicType>>
class C {}

class D {}

<<__SupportDynamicType>>
trait T1 {
  require extends C;
}

<<__SupportDynamicType>>
trait T2 {
  require extends D;
}

trait T3 {
  require extends C;
}
