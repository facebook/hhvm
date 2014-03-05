<?php

$code = <<<'EOF'
<?php
if (1) {} elseif (2) {}
EOF;

$serializedAST = HH\CodeModel\get_code_model_for($code);
$ast = unserialize($serializedAST);
$formatter = new HH\CodeModel\CodeModelToPHP();
echo $formatter->visitScript($ast, "php");
