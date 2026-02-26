<?hh

interface IBar { public function f1() : void; }
interface IBuzz extends IBar {public function f2() : void;}
class Baz implements IBuzz {
  public function f1() : void {}
  public function f2() : void {}
}

<<__ConsistentConstruct>>
abstract class A<T as IBuzz as IBar> {
  public function foo() : T {
    return $this->e;
  }

  public function __construct(protected T $e) {}
}

class D extends A { }

<<__EntryPoint>>
function main() : void {
  var_dump(new D(new Baz())->foo());
}
