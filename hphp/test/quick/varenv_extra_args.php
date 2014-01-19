<?php

function foo($asd) {
  $y = "a";
  $$y = 12;
  var_dump(func_get_args());
  var_dump(func_get_arg(2));
}
foo(2, "foo", "bar");
