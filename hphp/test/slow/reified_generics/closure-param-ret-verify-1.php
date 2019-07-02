<?hh

function f<reify T1, reify T2>() {
  return (T1 $x): T2 ==> {
    return $x;
  };
}

<<__EntryPoint>>
function main() {
  set_error_handler(
  (int $errno,
  string $errstr,
  string $errfile,
  int $errline,
  array $errcontext
  ) ==> {
    echo "ERROR: "; var_dump($errstr);
    throw new Exception();
  });
  $fn1 = f<int, int>();
  var_dump($fn1(1));
  try { var_dump($fn1("hi")); } catch (Exception $_) {}
  $fn2 = f<string, int>();
  try { var_dump($fn2("hi")); } catch (Exception $_) {}
}
