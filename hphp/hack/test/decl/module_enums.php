//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module X {}
new module Y {}

//// X.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module X;

internal enum X: int {
  A = 0;
  B = 1;
  C = 2;
}


internal function f1(X $x): void {} // ok


function f2(X $x): void {} // error


internal function f5(): void {
  $x = X::A; // ok
}


function f6(): void {
  $x = X::A; // ok
}

//// Y.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module Y;


function f3(X $x): void {} // error



function f7(): void {
  $x = X::A; // error
}

//// no-module.php
<?hh

function f4(X $x): void {} // error

function f8(): void {
  $x = X::A; // error
}
