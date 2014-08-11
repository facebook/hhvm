<?php
$doc = new DOMDocument();
$result = $doc->loadHTMLFile(dirname(__FILE__) . "/not_well.html");
assert('$result === true');
?>
