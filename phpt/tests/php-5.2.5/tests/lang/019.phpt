--TEST--
eval() test
--FILE--
<?php 

error_reporting(0);

eval("function test() { echo \"hey, this is a function inside an eval()!\\n\"; }");

$i=0;
while ($i<10) {
  eval("echo \"hey, this is a regular echo'd eval()\\n\";");
  test();
  $i++;
}
