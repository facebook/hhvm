<?hh


<<__EntryPoint>>
function main_805() :mixed{
$collections = dict[
  'Vector' => new Vector(),
  'Map' => new Map(),
  'Set' => new Set(),
];
try {
  foreach ($collections as $cls => $obj) {
    try {
      $x = $obj->foo;
      echo "$cls::get: ";
      var_dump($x);
    }
    catch (RuntimeException $e) {
      echo "$cls::get throws\n";
    }
    try {
      $x = isset($obj->foo);
      echo "$cls::isset: ";
      var_dump($x);
    }
    catch (RuntimeException $e) {
      echo "$cls::isset throws\n";
    }
    try {
      $obj->foo = 123;
    } catch (RuntimeException $e) {
      echo "$cls::set throws\n";
    }
  }
}
 catch (Exception $e) {
  echo "Fail!\n";
}
echo "Done\n";
}
