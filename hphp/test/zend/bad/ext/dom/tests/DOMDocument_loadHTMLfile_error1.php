<?php
$doc = new DOMDocument();
$result = $doc->loadHTMLFile(dirname(__FILE__) . "/ffff/test.html");
assert('$result === false');
?>
