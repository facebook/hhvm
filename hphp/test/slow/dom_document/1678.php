<?php
class foo {
  function foo($a) {
    var_dump($a);
  }
}


<<__EntryPoint>>
function main_1678() {
$xml =  '<root>$1 - <template><title>SITENAME</title></template></root>';
$dom = new DOMDocument();
$dom->loadXML($xml);
new foo($dom->documentElement);
}
