<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class IndexMismatch {
  public function foo() : void {
    $v = vec[];
    echo $v["nine"];
  }
}
