<?hh

function foo1<T as num>(T $x): T {
  return $x;
}

function foo2<T as int>(): T {
  return 'a';
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
}
