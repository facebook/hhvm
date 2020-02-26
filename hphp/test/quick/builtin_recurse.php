<?hh

function foo($s) {
  print "In foo(\"".$s."\")\n";
  throw new Exception("foo exception");
}

<<__EntryPoint>> function main(): void {
  print "Test begin\n";

  call_user_func_array(fun("print_r"), varray["Hello!\n"]);
  call_user_func_array(fun("call_user_func_array"), varray["print_r",
                                                     varray["Hello!\n"]]);

  # Throw an exception throw nested builtin calls.
  try {
    call_user_func_array(fun("call_user_func_array"), varray["foo", varray["foo arg"]]);
  } catch (Exception $e) {
    print "Caught exception\n";
  }

  print "Test end\n";
}
