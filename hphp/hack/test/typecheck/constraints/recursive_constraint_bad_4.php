<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class MySet<Tx as Ty, Ty as Tx> {
  public function FromTxToInt(Tx $x): int {
    return $x;
  }
}
