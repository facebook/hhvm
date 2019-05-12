<?hh

class A {
  private static $priv = 7;
  function readVar() {
    A::$priv;
  }
}

<<__EntryPoint>> function main(): void {
  (new A())->readVar();
  var_dump(isset(A::$priv));
}
