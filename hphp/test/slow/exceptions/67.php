<?hh

function runstuff() {
  throw new Exception;
}
function main() {
    try {
      $start_time = 1;
      runstuff();
      return;
    }
 catch (Exception $se) {
      $elapsed = 2 - $start_time;
      var_dump($elapsed);
    }
}

<<__EntryPoint>>
function main_67() {
main();
}
