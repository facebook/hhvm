<?hh

function foo($s) :mixed{
  print "In foo(\"".$s."\")\n";
  throw new Exception("foo exception");
}

<<__EntryPoint>> function main(): void {
  print "Test begin\n";

  call_user_func_array(print_r<>, vec["Hello!\n"]);
  call_user_func_array(call_user_func_array<>,
                       vec[print_r<>, vec["Hello!\n"]]);

  // Throw an exception throw nested builtin calls.
  try {
    call_user_func_array(call_user_func_array<>, vec[foo<>, vec["foo arg"]]);
  } catch (Exception $e) {
    print "Caught exception\n";
  }

  print "Test end\n";
}
