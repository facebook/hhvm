<?hh

class Foo {
  public function __toString() :mixed{ some(); }
}

function some() :mixed{
  throw new Exception('bye');
}

function wat() :mixed{
  $z = 'asd' . mt_rand();
  $k = new Foo();
  $l = vec[$z, $z];
  $y = $k == $z;
  $k = null;
  return $y;
}
<<__EntryPoint>> function main(): void {
try { wat(); }
catch (Exception $ex) { echo "Caught: " . $ex->getMessage() . "\n"; }
echo "done\n";
}
