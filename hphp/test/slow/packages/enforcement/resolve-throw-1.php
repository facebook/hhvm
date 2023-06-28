<?hh

module a.b;

<<__EntryPoint>>
function main_resolve_throw_1() :mixed{
  try {
    baz1();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  $f = baz2();
  try {
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  $f = baz3();
  try {
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    baz4();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  $f = baz5();
  try {
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  $c = cbaz1();
  new $c;
  try {
    $c::foo();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  $c = cbaz2();
  try {
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  cbaz3();
}
