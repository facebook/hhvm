<?hh

class Foo {
  public function __toString() { some(); }
}

function some() {
  throw new Exception('bye');
}

function wat() {
  $z = 'asd' . mt_rand();
  $k = new Foo();
  $l = varray[$z, $z];
  $y = $k == $z;
  $k = null;
  return $y;
}
<<__EntryPoint>> function main(): void {
try { wat(); }
catch (Exception $ex) { echo "Caught: " . $ex->getMessage() . "\n"; }
echo "done\n";
}
