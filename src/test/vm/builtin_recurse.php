<?php

function main() {
  print "Test begin\n";

  call_user_func_array("print_r", array("Hello!\n"));
  call_user_func_array("call_user_func_array", array("print_r",
                                                     array("Hello!\n")));

# Throw an exception throw nested builtin calls.
  function foo($s) {
    print "In foo(\"".$s."\")\n";
    throw new Exception("foo exception");
  }
  try {
    call_user_func_array("call_user_func_array", array("foo", array("foo arg")));
  } catch (Exception $e) {
    print "Caught exception\n";
  }

  print "Test end\n";
}

main();
