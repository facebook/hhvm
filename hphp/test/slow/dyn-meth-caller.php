<?hh

class Foo {
  function inst_meth() { echo __FUNCTION__."\n"; }
  static function static_meth() { echo __FUNCTION__."\n"; }
}

<<__EntryPoint>>
function main() {
  $a = hh\dynamic_meth_caller(Foo::class, 'inst_meth');
  $b = hh\dynamic_meth_caller(Foo::class, 'static_meth');
  $c = hh\dynamic_meth_caller(Foo::class, 'not_a_meth');

  var_dump($a, $b, $c);
  try { $a(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $b(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $c(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
}
