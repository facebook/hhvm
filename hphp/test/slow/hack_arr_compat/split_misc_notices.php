<?hh

<<__EntryPoint>>
function main () :mixed{
  $a = darray["" => "empty string", 1 => "one", 0 => "zero"];
  try {
    var_dump($a + darray["more stuff" => "yep"]);
  } catch (Exception $e) {
    print($e->getMessage())."\n";
  }
}
