////file1.php
<?hh

<<file:__EnableUnstableFeatures('newtype_super_bounds')>>
newtype X super int = arraykey;

newtype Y super C = nonnull;

class C {}

////file2.php
<?hh

function test(int $i): X {
  return $i;
}

function testcls(C $c): Y {
  return $c;
}
