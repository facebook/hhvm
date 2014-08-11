<?php
$dom = new DOMDocument();

$attr = $dom->createAttribute('string');
echo get_class($attr);

?>
