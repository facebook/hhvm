<?php

register_shutdown_function(function() {
  $error = error_get_last();

  if($error['type'] & E_ERROR) {
    echo "Error\n";
  }
});

Foo::bar();
