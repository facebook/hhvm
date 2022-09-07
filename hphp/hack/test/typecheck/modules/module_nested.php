//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module Foo.Bar {}

//// foo.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>
module Foo.Bar;

class C1 {}

function F1(): void {}
