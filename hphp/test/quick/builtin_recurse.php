<?hh

function foo($s) {
  print "In foo(\"".$s."\")\n";
  throw new Exception("foo exception");
}

<<__EntryPoint>> function main(): void {
  print "Test begin\n";

  call_user_func_array(fun("print_r"), array("Hello!\n"));
  call_user_func_array(fun("call_user_func_array"), array("print_r",
                                                     array("Hello!\n")));

  # Throw an exception throw nested builtin calls.
  try {
    call_user_func_array(fun("call_user_func_array"), array("foo", array("foo arg")));
  } catch (Exception $e) {
    print "Caught exception\n";
  }

  print "Test end\n";
}
