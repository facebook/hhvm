<?hh

function foo1<T as num>(T $x): T {
  return $x;
}

function foo2<T as int>(): T {
  return 'a';
}

class Bar<T as string> {
  function foo3<T as num>(T $x): T {
    return $x;
  }
  function foo4(T $x): T {
    return strlen($x);
  }
}

<<__EntryPoint>>
function main() {
  set_error_handler(
    (int $errno, string $errstr) ==> {
      throw new Exception($errstr);
    }
  );
  try {
    foo1('a');
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    foo2();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  $o = new Bar;
  try {
    $o->foo3('a');
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $o->foo4('a');
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
