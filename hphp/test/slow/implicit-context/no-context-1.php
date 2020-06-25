<?hh

include 'implicit.inc';

<<__NoContext>>
function f() {
  echo "Failure!\n";
}

<<__EntryPoint>>
function main() {
  try {
    ClassContext::start(new C, fun('f'));
    echo "Failure2!\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
