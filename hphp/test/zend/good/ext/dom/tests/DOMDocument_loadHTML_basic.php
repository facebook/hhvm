<?php
$doc = new DOMDocument();
$doc->loadHTML("<html><body><p>Test<br></p></body></html>");
echo $doc->saveHTML();
?>
