<?hh
function pp_exn(Exception $e) :mixed{
  echo $e->getMessage() . " @ " . $e->getFile() . ":" . $e->getLine() . "\n";
}

<<__EntryPoint>>
function foo(): void {
  include "instance-properties.inc";
  ok();
  try {
    $x = Foo::$y; // error
    var_dump($x);
  } catch (Exception $e) {
    pp_exn($e);
  }
  try {
    Foo::$y = 7; // error
  } catch (Exception $e) {
    pp_exn($e);
  }
  try {
    Foo::$y++; // error
  } catch (Exception $e) {
    pp_exn($e);
  }

  try {
    Foo::$y--; // error
  } catch (Exception $e) {
    pp_exn($e);
  }

  try {
    Foo::$sv[] = 5; // error
  } catch (Exception $e) {
    pp_exn($e);
  }
  try {
    $z = Foo::$sv[0]; // error
    var_dump($z);
  } catch (Exception $e) {
    pp_exn($e);
  }
  try {
    isset(Foo::$sv);
  } catch (Exception $e) {
    pp_exn($e);
  }
  try {
    isset(Foo::$sv[0]);
  } catch (Exception $e) {
    pp_exn($e);
  }
}
