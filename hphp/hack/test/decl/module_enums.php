//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module X {}
module Y {}

//// X.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module("X")>>

<<__Module("X"), __Internal>>
enum X: int {
  A = 0;
  B = 1;
  C = 2;
}

<<__Module("X"), __Internal>>
function f1(X $x): void {} // ok

<<__Module("X")>>
function f2(X $x): void {} // error

<<__Module("X"), __Internal>>
function f5(): void {
  $x = X::A; // ok
}

<<__Module("X")>>
function f6(): void {
  $x = X::A; // ok
}

//// Y.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module("Y")>>

<<__Module("Y")>>
function f3(X $x): void {} // error


<<__Module("Y")>>
function f7(): void {
  $x = X::A; // error
}

//// no-module.php
<?hh

function f4(X $x): void {} // error

function f8(): void {
  $x = X::A; // error
}
