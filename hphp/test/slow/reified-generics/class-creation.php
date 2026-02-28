<?hh

class C {}
class D {}
class E {}

class Test<reify T1> {
  public function f<reify T2>(bool $fun) :mixed{
    return $fun ? new T2 : new T1;
  }
}

function f<reify T>() :mixed{
  return new T();
}
<<__EntryPoint>> function main(): void {
echo "function\n";

var_dump(f<C>() is C);
var_dump(f<D>() is C);

echo "\nclass\n";

$t = new Test<E>();
var_dump($t->f<C>(true) is C);
var_dump($t->f<D>(true) is C);
var_dump($t->f<C>(false) is C);
var_dump($t->f<D>(false) is C);
var_dump($t->f<D>(true) is E);
var_dump($t->f<D>(false) is E);
}
