<?hh

class Foo {
  <<__DynamicallyCallable>> function inst_dyn() { echo __FUNCTION__."\n"; }
  function inst_meth() { echo __FUNCTION__."\n"; }
  static function static_meth() { echo __FUNCTION__."\n"; }
}

<<__EntryPoint>>
function main() {
  $z = HH\dynamic_meth_caller(Foo::class, 'inst_dyn');
  $a = HH\dynamic_meth_caller(Foo::class, 'inst_meth');
  $b = HH\dynamic_meth_caller(Foo::class, 'static_meth');
  $c = HH\dynamic_meth_caller(Foo::class, 'not_a_meth');

  var_dump($z, $a, $b, $c);
  try { $z(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $a(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $b(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $c(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
}
