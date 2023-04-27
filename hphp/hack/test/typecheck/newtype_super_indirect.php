////file1.php
<?hh

<<file:__EnableUnstableFeatures('newtype_super_bounds')>>
newtype X super int = arraykey;

////file2.php
<?hh

<<file:__EnableUnstableFeatures('newtype_super_bounds')>>
newtype Y as X = int;

////file3.php
<?hh

function test(Y $y): X {
  return $y;
}
