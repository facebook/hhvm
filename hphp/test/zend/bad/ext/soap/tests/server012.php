<?php
parse_str("WSDL", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

function Add($x,$y) {
  return $x+$y;
}

$server = new soapserver(null,array('uri'=>"http://testuri.org"));
$server->addfunction("Add");
$server->handle();
echo "ok\n";
?>