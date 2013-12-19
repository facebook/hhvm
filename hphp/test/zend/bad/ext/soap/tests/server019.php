<?php
function test() {
  return "Hello World";
}

$server = new soapserver(null,array('uri'=>"http://testuri.org"));
$server->addfunction("test");
$server->handle();
echo "ok\n";
?>