<?hh

interface Fooable {}
interface Barable {}

class Meh {}

function foo1<T as Fooable as Barable>(T $x): T {
  return $x;
}

function foo2<T as Fooable as Barable>(): T {
  return new Meh;
}

function foo3<T as num as int>(T $x): T {
  return $x;
}

function foo4<T as int as num>(T $x): T {
  return $x;
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(
    (int $errno, string $errstr) ==> {
      throw new Exception($errstr);
    }
  );
  try {
    foo1(new Meh);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    foo2();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    foo3(3.14);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  foo3(3);
  try {
    foo3('a');
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    foo4(3.14);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  foo4(3);
  try {
    foo4('a');
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
