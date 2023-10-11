<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function FromTxToInt<T1, T2, T3, T4, Tx as Ty, Ty as Tx>(Tx $x): int {
  return $x;
}
