<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T1<Ta, Tb as Ta> {
  final public function do(Tb $in): Ta {
    return $in;
  }
}
abstract class A1 {
  use T1<this::TA, this::TB>;
  abstract const type TA;
  abstract const type TB;
}

class C1{
  use T1<string,int>;
}
