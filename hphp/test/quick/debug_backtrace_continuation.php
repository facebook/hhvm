<?php
function my_generator() {
  $value = yield null;
  var_dump(debug_backtrace());
}

function my_wrapper() {
  $gen = my_generator();
  $gen->next();
  $gen->send(null);
}

my_wrapper();
