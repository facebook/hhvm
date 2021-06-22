<?hh

class A { public static function meth() {} }

function LV($x) { return __hhvm_intrinsics\launder_value($x); }

function wrap($fun) {
  try {
    $fun();
  } catch (Exception $e) {
    echo "Caught: {$e->getMessage()}\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $x = LV(HH\class_meth(A::class, 'meth'));

  wrap(() ==> var_dump((string)$x));
  wrap(() ==> var_dump((bool)$x));
  wrap(() ==> var_dump((float)$x));
  wrap(() ==> var_dump((int)$x));
  wrap(() ==> var_dump(vec($x)));
  wrap(() ==> var_dump(dict($x)));
  wrap(() ==> var_dump(keyset($x)));
}
