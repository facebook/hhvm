<?hh

class Bar<reify Tc as num> {
  function baz(Tc $x) :mixed{
    var_dump($x);
  }
}

function foo<reify T as num>(T $x) :mixed{
  var_dump($x);
}

function foobar<reify T as num>(T $x): T {
  var_dump($x);
  return 'a';
}

<<__EntryPoint>> function main() :mixed{
  set_error_handler(
    (int $errno, string $errstr) ==> {
     if ($errno == E_WARNING) {
        var_dump($errstr);
      } else {
        throw new Exception($errstr);
      }
    }
  );

  try {
    foo<string>('a'); // reify check passes, ub fails
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  $o = new Bar<string>;
  try {
    $o->baz('a'); // reify check passes, ub fails
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    foo<string>(1); // reify check fails, ub passes
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $o->baz(1); // reify check fails, ub passes
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    foobar<int>(1); // both passes for param, both fails for return
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
