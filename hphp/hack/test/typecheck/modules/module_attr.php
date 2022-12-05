//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module here {}
new module there {}
new module another {}

//// here.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>
module here;

function foo(): void { }

type Talias = int;

newtype Tnew = string;

//// there.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>
module there;

class C {
  public function bar(): void { }
}

//// another.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>
module another;

enum E : int {
  A = 3;
}
