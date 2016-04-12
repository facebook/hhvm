<?php

class Foo {
  public static function bar() {
    var_dump(Foo::class);
    var_dump(foo::class);
    var_dump(self::class);
    var_dump(Foo::CLASS);
    var_dump(foo::ClAsS);
    var_dump(seLF::CLass);
  }
}

class Baz extends Foo {
  public function qux() {
    var_dump(parENT::CLAss);
    var_dump(staTIC::claSS);
  }
}

foo::bar();
$b = new Baz;
$b->qux();
