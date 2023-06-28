<?hh

abstract class A {
  abstract const type T1;
  abstract const type T2 = int;
  const type T3 = int;

  abstract const int V1;
  const int V2 = 1;

  abstract const ctx C1;
  abstract const ctx C2 = [];
  const ctx C3 = [];
}

<<__EntryPoint>>
function main() :mixed{
  $a = new ReflectionClass('A');
  foreach ($a->getTypeConstants() as $tc) {
    echo $tc->getName()
         . ' '
         . ($tc->isAbstract() ? "abstract" : "not abstract")
         . "\n";
  }
  var_dump($a->getConstants());
}
