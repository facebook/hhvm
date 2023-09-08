//// modules.php
<?hh


new module here {}
new module there {}
new module another {}

//// here.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module here;

function foo(): void { }

type Talias = int;

newtype Tnew = string;

//// there.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module there;

class C {
  public function bar(): void { }
}

//// another.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module another;

enum E : int {
  A = 3;
}
