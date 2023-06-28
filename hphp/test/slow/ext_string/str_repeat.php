<?hh


<<__EntryPoint>>
function main_str_repeat() :mixed{
  try {
    $var = str_repeat('A', -1);
    var_dump($var);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  $var = str_repeat('A', 9223372036854775807);
  var_dump($var);
}
