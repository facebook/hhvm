<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class MySet<Tx as Ty, Ty as Tx> {
  public function FromTxToTy(Tx $x): Ty {
    return $x;
  }
  public function FromTyToTx(Ty $y): Tx {
    return $y;
  }
}
