<?hh

<<__EntryPoint>>
function main() :mixed{
  $a = dict["42" => 10];
  var_dump($a);

  $a["10"] = 5;
  var_dump($a);

  try { var_dump($a[42]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  var_dump($a["42"]);

  $b = darray(dict["42" => "string", 42 => "int"]);
  var_dump($b);
  var_dump($b["42"]);
  var_dump($b[42]);
  var_dump(dict($b));
  var_dump(vec($b));
}
