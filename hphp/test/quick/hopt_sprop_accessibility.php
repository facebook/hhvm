<?hh

class A {
  private static $priv = 7;
  function readVar() :mixed{
    return A::$priv;
  }
}
<<__EntryPoint>>
function entrypoint_hopt_sprop_accessibility(): void {

  $nonstaticUnscoped = function () {
    var_dump(A::$priv);
  };

  (new A())->readVar();
  $nonstaticUnscoped();
}
