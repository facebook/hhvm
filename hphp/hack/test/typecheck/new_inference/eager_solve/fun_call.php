<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(): void {
  $fs = Vector {() ==> {}};
  foreach ($fs as $f) {
    $f();
  }
}
