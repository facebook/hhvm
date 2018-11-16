<?hh

class C {}
class D {}
class E {}

class Test<reify T1> {
  public function f<reify T2>(bool $fun) {
    return $fun ? new T2 : new T1;
  }
}

function f<reify T>() {
  return new T();
}

echo "function\n";

var_dump(f<reify C>() is C);
var_dump(f<reify D>() is C);

echo "\nclass\n";

$t = new Test<reify E>();
var_dump($t->f<reify C>(true) is C);
var_dump($t->f<reify D>(true) is C);
var_dump($t->f<reify D>(false) is C);
var_dump($t->f<reify D>(false) is C);
var_dump($t->f<reify D>(true) is E);
var_dump($t->f<reify D>(false) is E);
