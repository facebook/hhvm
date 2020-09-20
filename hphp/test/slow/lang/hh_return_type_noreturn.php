<?hh
function handler($errno, $errmsg) {
  if ($errno === E_RECOVERABLE_ERROR) {
    throw new Exception($errmsg);
  } else if ($errno === E_WARNING) {
    echo "Triggered E_WARNING: $errmsg\n";
  } else if ($errno === E_NOTICE) {
    echo "Triggered E_NOTICE: $errmsg\n";
  } else {
    return false;
  }
}

function return_implicit(): noreturn {}

function return_on_its_own(): noreturn { return; }

function call_wrapper($f) {
  try {
    $f();
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}

function main() {
  call_wrapper(fun('return_implicit'));
  call_wrapper(fun('return_on_its_own'));
  echo 'Done', "\n";
}


<<__EntryPoint>>
function main_hh_return_type_noreturn() {
error_reporting(-1);
set_error_handler(fun('handler'));

main();
}
