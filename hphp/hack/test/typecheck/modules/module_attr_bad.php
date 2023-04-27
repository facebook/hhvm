//// modules.php
<?hh


new module here {}

//// main.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module here;

function foo():void { }

class C {
  public function bar():void { }
}

type Talias = int;

enum E : int {
  A = 3;
}

newtype Tnew = string;
