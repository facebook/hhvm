<?hh

class A {
  protected $x = 1;
}

class B extends A {
  protected $x = 2;
}

class C extends A {
  public function test($obj) :mixed{
    var_dump($obj->x);
  }
}

<<__EntryPoint>> function main(): void {
  $b = new B;
  $c = new C;
  $c->test($b);
}
