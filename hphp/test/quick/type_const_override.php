<?hh

interface IX {
}

class X implements IX {
}

interface IGP {
  abstract const type TType as IX;
  public function foo() : this::TType;
}

interface IP extends IGP {
  const type TType = IX;
}

abstract class TestingParent implements IGP {
}

final class TestingChild extends TestingParent implements IP {
  const type TType = IX;
  <<__Override>>
  public function foo() : this::TType {
    return new X();
  }
}

<<__EntryPoint>>
function main() :mixed{
  $o = new TestingChild;
  var_dump($o->foo());
}
