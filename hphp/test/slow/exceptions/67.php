<?hh

function runstuff() :mixed{
  throw new Exception;
}
function main() :mixed{
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
function main_67() :mixed{
main();
}
