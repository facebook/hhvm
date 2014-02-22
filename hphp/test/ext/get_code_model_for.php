<?php
$code = '<?php
$global = 1;

function foo((function(): int) $x) {
  return $x + 1;
}

function bar(<<someAttribute>> $one) {
}
';

$serializedAST = HH\CodeModel\get_code_model_for($code);
$ast = unserialize($serializedAST);
$formatter = new HH\CodeModel\CodeModelToPHP();
echo $formatter->visitScript($ast, "php");
echo "\n";
