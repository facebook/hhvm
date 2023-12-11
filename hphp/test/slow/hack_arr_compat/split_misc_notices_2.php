<?hh

<<__EntryPoint>>
function main () :mixed{
  $a = dict["" => "empty string", 1 => "one", 0 => "zero"];
  try {
    var_dump($a + dict["more stuff" => "yep"]);
  } catch (Exception $e) {
    print($e->getMessage()."\n");
  }
}
