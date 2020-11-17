<?hh

<<__NoContext>>
function f() {
  echo "Failure!\n";
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';
  try {
    ClassContext::start(new C, f<>);
    echo "Failure2!\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
