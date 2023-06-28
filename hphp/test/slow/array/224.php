<?hh


<<__EntryPoint>>
function main_224() :mixed{
$array_variables = varray[  varray[],  varray[NULL],  varray[]];
foreach ($array_variables as $array_var) {
  $keys = array_keys($array_var);
  foreach ($keys as $key_value) {
    echo $key_value;
  }
}
}
