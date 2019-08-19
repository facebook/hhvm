<?hh

function handler($errno, $errstr, $errfile, $errline) {
  echo $errstr."\n";
  throw new Exception("Failed to generate code coverage");
}

function foo() {
  echo "I'm FOOO\n";
}

<<__EntryPoint>>
function main() {
  set_error_handler("handler");
  if (isset($_GET['enable_code_coverage'])) {
    echo "Code coverage: ".$_GET['enable_code_coverage']."\n";
  }

  try {
    fb_enable_code_coverage();
    foo();
    var_dump(fb_disable_code_coverage());
  } catch (Exception $e) {
    var_dump($e->getmessage());
  }
}
