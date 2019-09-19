<?hh

class c {
}

function my_handler() {
  throw new Exception('whoops');
}



function main() {
  echo "Entering try\n";
  try {
    echo "Creating first c\n";
    $o = new c;
    echo "Creating second c\n";
    $p = new c;
    $o1 = $o2 = $o3 = $o4 = $o5 = $o6 = $o; // fill up caller-saved
    $o7 = $o; // use callee saved
    $p = 5;
    echo $x;
  } catch (Exception $e) {
    echo "Caught: " . $e->getMessage() . "\n";
  }
  echo "Returning from main\n";
}
<<__EntryPoint>> function main_entry() {
set_error_handler(fun('my_handler'));
echo "Calling main()\n";
main();
echo "Back from main()\n";
}
