<?php

$xml = new SimpleXmlElement("<img></img>");
$xml->addAttribute("src", "foo");
$xml->addAttribute("alt", "");
echo $xml->asXML();

?>
===DONE===