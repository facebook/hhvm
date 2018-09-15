<?hh

class C {}
class D {}
class E {}

class One<reified T1> {
  public function f<reified T2>(bool $fun) {
    return $fun ? new T2 : new T1;
  }
}

class Two<reified T1> {
  public function f<reified T2>(bool $fun) {
    $c = new One<reified T1>();
    return $c->f<reified T2>($fun);
  }
}

$t = new Two<reified E>();
var_dump($t->f<reified C>(true) is C);
var_dump($t->f<reified C>(true) is D);
var_dump($t->f<reified C>(true) is E);
var_dump($t->f<reified C>(false) is C);
var_dump($t->f<reified C>(false) is D);
var_dump($t->f<reified C>(false) is E);
