<?hh

class C {}
class D {}
class E {}

class Test<reified T1> {
  public function f<reified T2>(bool $fun) {
    return $fun ? new T2 : new T1;
  }
}

function f<reified T>() {
  return new T();
}

echo "function\n";

var_dump(f<reified C>() is C);
var_dump(f<reified D>() is C);

echo "\nclass\n";

$t = new Test<reified E>();
var_dump($t->f<reified C>(true) is C);
var_dump($t->f<reified D>(true) is C);
var_dump($t->f<reified D>(false) is C);
var_dump($t->f<reified D>(false) is C);
var_dump($t->f<reified D>(true) is E);
var_dump($t->f<reified D>(false) is E);
