<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function FromTxToTy<Tx as Ty, Ty as Tx>(Tx $x): Ty {
  return $x;
}
function FromTyToTx<Tx as Ty, Ty as Tx>(Ty $y): Tx {
  return $y;
}
