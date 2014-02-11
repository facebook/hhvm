<?php
$code = '<?php
$global = 1;

function foo($x) {
  return $x + 1;
}
';

$serializedAST = HH\CodeModel\get_code_model_for($code);
$ast = unserialize($serializedAST);
$formatter = new HH\CodeModel\CodeModelToPHP();
echo $formatter->visitScript($ast, "php");
echo "\n";
