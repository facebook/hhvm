<?php
$doc = new DOMDocument();
$result = $doc->loadHTMLFile(dirname(__FILE__) . "/empty.html");
assert('$result === true');
?>
