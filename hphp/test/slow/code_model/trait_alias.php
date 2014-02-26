<?php

$code = '<?php
class A {
  use T {
    foo as bar;
  }
}
';

$serializedAST = HH\CodeModel\get_code_model_for($code);
$ast = unserialize($serializedAST);

if ($ast) echo "OK!\n";
else echo "Error";
