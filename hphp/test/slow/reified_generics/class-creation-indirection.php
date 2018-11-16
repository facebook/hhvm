<?hh

class C {}
class D {}
class E {}

class One<reify T1> {
  public function f<reify T2>(bool $fun) {
    return $fun ? new T2 : new T1;
  }
}

class Two<reify T1> {
  public function f<reify T2>(bool $fun) {
    $c = new One<reify T1>();
    return $c->f<reify T2>($fun);
  }
}

$t = new Two<reify E>();
var_dump($t->f<reify C>(true) is C);
var_dump($t->f<reify C>(true) is D);
var_dump($t->f<reify C>(true) is E);
var_dump($t->f<reify C>(false) is C);
var_dump($t->f<reify C>(false) is D);
var_dump($t->f<reify C>(false) is E);
