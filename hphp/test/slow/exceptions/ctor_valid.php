<?php

function info(Exception $e) {
  var_dump($e->getMessage());
  var_dump($e->getCode());
}

echo "Paramless\n";
info(new Exception());

echo "Defaults\n";
info(new Exception('', 0, null));

echo "String message\n";
info(new Exception('test'));

echo "Integer message\n";
info(new Exception(42));

echo "Float message\n";
info(new Exception(42.42));

echo "Boolean message\n";
info(new Exception(true));

echo "Null message\n";
info(new Exception(null));

echo "__toString message\n";
class ExceptionArg {
  function __toString() { return 'arg'; }
}
info(new Exception(new ExceptionArg()));

echo "Integer code\n";
info(new Exception('test', 42));

echo "Float code\n";
info(new Exception('test', 42.42));

echo "Boolean code\n";
info(new Exception('test', true));

echo "Null code\n";
info(new Exception('test', null));

echo "Previous\n";
$prevE = new Exception();
info($e = new Exception('test', 42, $prevE));
var_dump($e->getPrevious() === $prevE);
