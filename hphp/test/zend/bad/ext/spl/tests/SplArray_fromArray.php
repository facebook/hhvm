<?php
$array = array(PHP_INT_MAX => 'foo');
$splArray = new SplFixedArray();

try {
  $splArray->fromArray($array);
} catch (Exception $e) {
  echo $e->getMessage();
}
?>