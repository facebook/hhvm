<?php

function helper($arg) {
  var_dump(xdebug_get_function_stack());
}

helper(10000);
