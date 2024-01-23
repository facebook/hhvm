<?hh

abstract final class FStatics {
  public static $count = 1;
}
function f() :mixed{
  throw new Exception((string)FStatics::$count);
}
function test($fn) {
  try { $fn(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage()."\n"; }
}
// From PHP 7 testing sources, with minor modification

<<__EntryPoint>>
function main_preg_replace_callback_array_errors(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  $count = -1;
  test(() ==> var_dump(preg_replace_callback_array()));
  test(() ==> var_dump(preg_replace_callback_array(1)));
  test(() ==> var_dump(preg_replace_callback_array(1, 2, -1, inout $count)));
  test(() ==> var_dump(preg_replace_callback_array(1,2,3, inout $count)));
  // Provide an integer subject; no warning, just null
  var_dump(preg_replace_callback_array(vec[], 3, -1, inout $count));
  $a = 5;
  test(() ==> var_dump(preg_replace_callback_array(1, 2, 3, inout $a)));
  $a = "";
  test(() ==> var_dump(preg_replace_callback_array(dict["" => ""], "", "", inout $a)));
  $a = vec[];
  $b = "";
  test(() ==> var_dump(preg_replace_callback_array($a, $a, $a, inout $a, $b)));
  var_dump($b);
  $b = -1;
  // PHP 7 used preg_replace_callback here, but we are testing
  // preg_replace_callback_array.
  // Testing multiple invalid. We only catch the first invalid one
  // even if there are more - that matches PHP 7
  test(() ==> var_dump(preg_replace_callback_array(
    dict["xx" => "notValid1", "yy" => "notValid2"], $a, -1,
    inout $b,
  )));
  test(() ==> var_dump(preg_replace_callback_array(dict['/\w' => 'f'], 'z', -1, inout $count)));
  try {
    var_dump(
      preg_replace_callback_array(
        dict['/\w/' => 'f', '/.*/' => 'f'],
        'z',
        -1,
        inout $count
      )
    );
  } catch (Exception $e) {
    // This returns a string "1" in PHP 7; We return an int 1. That's
    // just a difference in our implementation of how we store exception
    // messages
    var_dump($e->getMessage());
  }
  echo "Done\n";
}
