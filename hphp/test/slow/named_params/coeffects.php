<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class A {
  public function f(named int $x = 0, named int $y = 1)[write_props] {
    $z = vec[$x, $y];
    return $z;
  }
}

<<__EntryPoint>>
function main() :mixed{
  $a = new A();
  var_dump($a->f());
  var_dump($a->f(x=100));
  var_dump($a->f(y=200));
  var_dump($a->f(x=100, y=200));
}
