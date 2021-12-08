//// here.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module("here")>>

function foo(): void { }

type Talias = int;

newtype Tnew = string;

//// there.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module("there")>>

class C {
  public function bar(): void { }
}

//// another.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module("another")>>

enum E : int {
  A = 3;
}
