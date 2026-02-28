<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class CAttr implements HH\ClassAttribute {
  public function __construct() {}
}

<<CAttr>>
function f(): void {}
