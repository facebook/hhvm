<?hh

<<__SoundDynamicCallable>>
class C {}

function f (dynamic $d) : dynamic {
  $c = new C();

  return $d->m(1, "x", 1 + 2, new C(), $c, vec[$c]);
}
