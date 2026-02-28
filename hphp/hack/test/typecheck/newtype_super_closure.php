////file1.php
<?hh

<<file:__EnableUnstableFeatures('newtype_super_bounds')>>
newtype X super int = arraykey;
newtype Y = arraykey;
////file2.php
<?hh

function hof<T>((function(T): void) $f, T $t): void {
  $f($t);
}

function testX(X $p): void {
  hof(
    (X $x) ==> testX($x),
    $p
  );
}

function testY(Y $q): void {
  hof(
    (Y $y) ==> testY($y),
    $q
  );
}
