<?php
/* Using Class and objects */
echo "\n*** Testing for object ***\n";
class classA
{
  public  $v;
}

$A = new classA();
$arr = get_object_vars($A);
var_dump ( extract(&$arr, EXTR_REFS));

echo "Done\n";
?>
