<?php

$code = '<?php
class A {
  use T {
    foo as bar;
    foo::u as y;
  }
}
';

$serializedAST = HH\CodeModel\get_code_model_for($code);
$ast = unserialize($serializedAST);
$formatter = new HH\CodeModel\CodeModelToPHP();
echo $formatter->visitScript($ast, "php");
