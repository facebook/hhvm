//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module Foo {}
//// foo.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules'), __Module('Foo')>>

<<__Module("lol")>>
final class Bar {}

function baz(): void {}
