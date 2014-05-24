<?php
$buffer = '';
function my_output($output, $flag) {
  global $buffer;
  $buffer = var_export(['output' => $output, 'flags' => $flag], true);
  return $output;
}

ob_start('my_output');
echo "herp";
ob_clean();
var_dump($buffer);
ob_start('my_output');
echo "derp";
ob_end_clean();
var_dump($buffer);
