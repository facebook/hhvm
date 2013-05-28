<?php

ob_start();
set_exception_handler('user_exception_handler');
echo 'Hello World';
throw new Exception;
function user_exception_handler($e) {
  ob_end_clean();
  var_dump(error_get_last());
  echo 'Goodbye';
  var_dump(error_get_last());
}
