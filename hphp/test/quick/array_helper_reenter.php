<?php

// This is a micro-test for a specific, tricky bug in the jit. See comments
// inline for explanation. It hinges on invoking the user error handler (i.e.
// reentering the VM) from one of the jit's array helpers, which are directly
// called into from jitted code.

function error_handler($errno, $msg, $file, $line) {
  echo "error handler\n";
}
set_error_handler('error_handler');

function f($form) {
  // The call to a builtin is needed to invoke the code that saves the VM's
  // hardware registers to ExecutionContext, and gives them values that will
  // be bogus below.
  if (!is_array(123)) {
    // The array-index operation calls into array_getm_s_fast, which will
    // raise a notice because the index is undefined. The runtime will attempt
    // to get a backtrace, which will crash because m_fp is out of sync and
    // points at something that isn't an ActRec.
    $sink = $form['default_value'];
  }
}

// It dies on the second invocation; not exactly sure why the first one is OK
f(array());
f(array());
