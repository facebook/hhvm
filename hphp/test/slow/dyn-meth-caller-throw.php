<?hh

class Foo {
  <<__DynamicallyCallable>> function inst_dyn() { echo __FUNCTION__."\n"; }
  function inst_meth() { echo __FUNCTION__."\n"; }
  static function static_meth() { echo __FUNCTION__."\n"; }
}

<<__EntryPoint>>
function main() {
  try { $z = HH\dynamic_meth_caller(Foo::class, 'inst_dyn'); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $a = HH\dynamic_meth_caller(Foo::class, 'inst_meth'); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $b = HH\dynamic_meth_caller(Foo::class, 'static_meth'); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $c = HH\dynamic_meth_caller(Foo::class, 'not_a_meth'); } catch (Exception $e) { var_dump($e->getMessage()); }

  try { var_dump($z);} catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump($a);} catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump($b);} catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump($c);} catch (Exception $e) { var_dump($e->getMessage()); }
  try { $z(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $a(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $b(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $c(new Foo); } catch (Exception $e) { var_dump($e->getMessage()); }
}
