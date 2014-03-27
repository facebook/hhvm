<?php
function handler ($errno, $errstr, $errfile, $errline, array $errcontext) {
  echo "handler_called\n";
}
set_error_handler('handler');
$undefined->foo();
