<?hh

abstract final class FStatics {
  public static $count = 1;
}
function f() {
  throw new Exception((string)FStatics::$count);
}


// From PHP 7 testing sources, with minor modification

<<__EntryPoint>>
function main_preg_replace_callback_array_errors() {
$count = -1;
try { var_dump(preg_replace_callback_array()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(preg_replace_callback_array(1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(preg_replace_callback_array(1, 2, -1, inout $count));
var_dump(preg_replace_callback_array(1,2,3, inout $count));
// Provide an integer subject; no warning, just null
var_dump(preg_replace_callback_array(varray[], 3, -1, inout $count));
$a = 5;
var_dump(preg_replace_callback_array(1, 2, 3, inout $a));
$a = "";
try { var_dump(preg_replace_callback_array(darray["" => ""], "", "", inout $a)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$a = varray[];
$b = "";
try { var_dump(preg_replace_callback_array($a, $a, $a, inout $a, $b)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump($b);
$b = -1;
// PHP 7 used preg_replace_callback here, but we are testing
// preg_replace_callback_array.
// Testing multiple invalid. We only catch the first invalid one
// even if there are more - that matches PHP 7
var_dump(preg_replace_callback_array(
  darray["xx" => "notValid1", "yy" => "notValid2"], $a, -1, inout $b)
);
var_dump($b);
var_dump(preg_replace_callback_array(darray['/\w' => 'f'], 'z', -1, inout $count));
try {
  var_dump(
    preg_replace_callback_array(
      darray['/\w/' => 'f', '/.*/' => 'f'],
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
