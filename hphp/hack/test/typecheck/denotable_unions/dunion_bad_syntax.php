<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function foo((int | bool & string) $ib):string {
  if ($ib is int) {
    return "is an int";
  } else if ($ib is bool) {
    return "is a bool";
  }
  return "cannot happen";
}
