<?hh

interface I {
  function foo():mixed;
}

trait T implements I {
  function foo() :mixed{ echo "Hello, ".static::class."!\n"; }
}

abstract class G implements I {}
class P extends G { use T; }
class C extends P {}

<<__EntryPoint>>
function main() :mixed{
  $m1 = meth_caller(I::class, "foo");
  $m2 = meth_caller(P::class, "foo");
  $m3 = meth_caller(C::class, "foo");
  $o1 = new P;
  $o2 = new C;

  $m1($o1); $m2($o1);
  try { $m3($o1); } catch (Exception $e) { var_dump($e->getMessage()); }

  $m1($o2); $m2($o2); $m3($o2);

  try {
    $mx = meth_caller(T::class, "foo");
    $mx($o1); $mx($o2);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
