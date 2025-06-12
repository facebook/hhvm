<?hh

class C {}

class :x {
  attribute
    classname<C> cn1 @required,
    ?classname<C> cn2 = null,
    class<C> c1 @required,
    ?class<C> c2 = null;

  function getAttribute($name) {
    var_dump($name);
  }
}

<<__EntryPoint>>
function main(): void {
  $x = <x cn1={nameof C} c1={C::class}></x>;
  $x->:c1;
}
