<?php
$doc = new DOMDocument();
$result = $doc->loadHTMLFile("");
assert('$result === false');
?>
