<?hh // in the default module (in the active deployment)

<<__EntryPoint>>
function main_throw_2() :mixed{
  try {
    foo();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    __hhvm_intrinsics\launder_value("foo")();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    new Foo();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  $c = __hhvm_intrinsics\launder_value("Foo");
  try {
    new $c;
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    Foo::foo();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    $c::foo();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    __hhvm_intrinsics\launder_value(vec[$c, "foo"])();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    __hhvm_intrinsics\launder_value("Foo::foo")();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    var_dump($c::x);
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  $c = __hhvm_intrinsics\launder_value(Foo::class);
  new $c;
}
