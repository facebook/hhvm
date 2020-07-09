<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T1<Ta, Tb as Ta> {
  final public function do(Tb $in): Ta {
    return $in;
  }
}
trait T2<Ta, Tb> {
  use T1<Ta, Tb>;
}
abstract class A1 {
  use T2<this::TA, this::TB>;
  abstract const type TA;
  abstract const type TB;
}

class C1{
  use T2<string,int>;
}
