<?hh

<<__EntryPoint>>
function main_error_depth() :mixed{
  $array = vec[];
  for ($i=0; $i<550; $i++) {
    $array = vec[$array];
  }

  $error = null;
  $result = json_encode_with_error($array, inout $error, 0, 551);
  var_dump($result);
  switch ($error[0] ?? 0) {
    case 0:
      echo 'OK'.PHP_EOL;
    break;
    case JSON_ERROR_DEPTH:
      echo 'ERROR'.PHP_EOL;
    break;
  }

  $error = null;
  $result = json_encode_with_error($array, inout $error, 0, 540);
  var_dump($result);
  switch ($error[0] ?? 0) {
    case 0:
      echo 'OK'.PHP_EOL;
    break;
    case JSON_ERROR_DEPTH:
      echo 'ERROR'.PHP_EOL;
    break;
  }
}
