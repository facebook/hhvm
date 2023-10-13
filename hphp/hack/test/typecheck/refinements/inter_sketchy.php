<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f(): void {
  $b = true;
  $x = ($b ? 0 : "");
  $x is bool ? ($x ? '' : '') : '';
}
