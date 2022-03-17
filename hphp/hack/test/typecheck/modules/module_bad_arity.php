//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module C {}
//// no-module.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
// TODO(T106200480)
<<file:__EnableUnstableFeatures('modules'), __Module>>

//// too-many-modules.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('A', 'B')>>

//// bad-internal.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('C')>>

<<__Internal(42)>>
function c(): void {}

class D {
  <<__Internal('lol')>>
  public function foobar(): void {}
}
