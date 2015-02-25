<?hh


function errorHandler($errno, $errstr, $errfile, $errline) {
  throw new Exception;
}

set_error_handler('errorHandler');

function rain() {
  $arr = array();
  for ($i = 0; $i < 4; $i++) {
    $arr[$i] = $i;
  }
  for ($i = 0; $i < 5; $i++) {
    print($arr[$i]."\n");
  }
}

function main() {
  try {
    rain();
  } catch(Exception $e) {
    print("Caught the rain\n");
  }
  rain();
  print("not_reached\n");
}

main();
