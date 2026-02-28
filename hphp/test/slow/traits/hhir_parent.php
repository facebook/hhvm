<?hh

class B {
  public static function bar() :mixed{
    echo "B\n";
  }
}

class D extends B {
  public static function bar() :mixed{
    echo "D\n";
  }
}

trait Yeah {
  public function foo() :mixed{
    // Bug #2339698.  Parent was skipping one.
    parent::bar();
  }
}

class C extends D {
  use Yeah;
}

function foo() :mixed{
  $k = new C();
  $k->foo();
}


<<__EntryPoint>>
function main_hhir_parent() :mixed{
foo();
}
