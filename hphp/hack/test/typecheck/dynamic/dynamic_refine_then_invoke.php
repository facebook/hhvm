////file1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function make_dynamic():dynamic {
  return 3 as dynamic;
}

////file2.php
<?hh
function foo():void {
  $d = make_dynamic();

  $x = $d ? $d->type : null;
  $y = $d->id;
}
