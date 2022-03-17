//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module a {}
module A {}
module B {}
//// a.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('a')>>

<<__Internal>>
function f(): void {}


//// A.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

function g(): void {
  f(); // ERROR: we are in `A`, not `a`
}

//// b.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module('b')>> // ERROR: no such module `b`

// TODO(T108206307) You _need_ a top level symbol to attach a module to in order
// to get an unbound module name error.
function h(): void {}
