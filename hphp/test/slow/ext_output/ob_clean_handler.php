<?php

function my_output($output, $flag) {

  ExtOutputObCleanHandler::$buffer = var_export(['output' => $output, 'flags' => $flag], true);
  return $output;
}

ob_start('my_output');
echo "herp";
ob_clean();
var_dump(ExtOutputObCleanHandler::$buffer);
ob_start('my_output');
echo "derp";
ob_end_clean();
var_dump(ExtOutputObCleanHandler::$buffer);

abstract final class ExtOutputObCleanHandler {
  public static $buffer = '';
}
