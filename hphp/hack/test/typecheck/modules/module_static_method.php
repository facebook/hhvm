//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

class A {
  <<__Internal>>
  public static function f(): void {}
}

function a(): void {
  A::f();
}

//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules'), __Module('B')>>

function b(): void {
  A::f();
}

//// none.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function none(): void {
  A::f();
}
