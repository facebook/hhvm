<?hh

error_reporting(-1);
function handler($errno, $errmsg) {
  if ($errno === E_RECOVERABLE_ERROR) {
    throw new Exception('E_RECOVERABLE_ERROR: $errmsg');
  } else if ($errno === E_WARNING || $errno === E_USER_WARNING) {
    throw new Exception('E_*WARNING: '.$errmsg);
  } else if ($errno === E_NOTICE || $errno === E_USER_NOTICE) {
    throw new Exception('E_*NOTICE: '.$errmsg);
  } else if ($errno === E_ERROR || $errno === E_USER_ERROR) {
    throw new Exception('E_*ERROR: '.$errmsg);
  } else {
    throw new Exception("Unexpected $errno: $errmsg");
  }
}
set_error_handler('handler');

function try_dynamic($r) {
  try {
    var_dump($r->dynamic);
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
  $r->dynamic = 'dynamic';
  var_dump($r->dynamic);
}

function reflect_method() {
  echo '= ', __FUNCTION__, ' =', "\n";
  $rm = new ReflectionMethod('reflectionclass', 'hasmethod');
  var_dump($rm->name);
  var_dump($rm->class);
  try_dynamic($rm);
  try {
    $rm->name = 'getMethod';
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
  try {
    $rm->class = 'ReflectionClass';
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}

function reflect_class() {
  echo '= ', __FUNCTION__, ' =', "\n";
  $rc = new ReflectionClass('reflectionclass');
  var_dump($rc->name);
  try_dynamic($rc);
  try {
    $rc->name = 'ReflectionMethod';
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}

function reflect_func() {
  echo '= ', __FUNCTION__, ' =', "\n";
  $rf = new ReflectionFunction('implode');
  var_dump($rf->name);
  try_dynamic($rf);
  try {
    $rf->name = 'explode';
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}

function main() {
  reflect_method();
  reflect_func();
  reflect_class();
}

main();
