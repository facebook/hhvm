<?hh

class A {
  private static $priv = 7;
  function readVar() :mixed{
    A::$priv;
  }
}

<<__EntryPoint>> function main(): void {
  (new A())->readVar();
  var_dump(isset(A::$priv));
}
