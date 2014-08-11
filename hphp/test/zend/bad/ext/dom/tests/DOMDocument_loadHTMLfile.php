<?php
$doc = new DOMDocument();
$result = $doc->loadHTMLFile(dirname(__FILE__) . "/test.html");
assert('$result === true');
?>
