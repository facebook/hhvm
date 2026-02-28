<?hh

class C {}
class D {}
class E {}

class One<reify T1> {
  public function f<reify T2>(bool $fun) :mixed{
    return $fun ? new T2 : new T1;
  }
}

class Two<reify T1> {
  public function f<reify T2>(bool $fun) :mixed{
    $c = new One<T1>();
    return $c->f<T2>($fun);
  }
}
<<__EntryPoint>> function main(): void {
$t = new Two<E>();
var_dump($t->f<C>(true) is C);
var_dump($t->f<C>(true) is D);
var_dump($t->f<C>(true) is E);
var_dump($t->f<C>(false) is C);
var_dump($t->f<C>(false) is D);
var_dump($t->f<C>(false) is E);
}
