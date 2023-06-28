<?hh

function main() :mixed{
  $v = vec[3, 2, 1];
  var_dump(array_key_exists(mt_rand(0,2), $v));
  var_dump(array_key_exists(mt_rand(count($v),PHP_INT_MAX), $v));
}


<<__EntryPoint>>
function main_array_key_exists() :mixed{
main();
}
