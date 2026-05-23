<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function f(int $x, int $y, named int $b, named int $a) :mixed{}

class C {
  public function m(string $p, named string $q, named string $r = 'default') :mixed{}
}

<<__EntryPoint>>
function main() :mixed{
  $rf = new ReflectionFunction('f');
  foreach ($rf->getParameters() as $param) {
    echo $param->getName() . ': ';
    var_dump($param->isNamed());
  }

  echo "\n";

  $rm = new ReflectionMethod('C', 'm');
  foreach ($rm->getParameters() as $param) {
    echo $param->getName() . ': ';
    var_dump($param->isNamed());
  }
}
