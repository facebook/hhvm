//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
interface A {}

interface A2 {}

interface A3 {
  <<__Internal>>
  public function f(): void;
  // Ok! But it's not possible to implement this outside the module
}

//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('B')>>

class B implements A {} // Bad

class B2 implements A2 {} // Ok
