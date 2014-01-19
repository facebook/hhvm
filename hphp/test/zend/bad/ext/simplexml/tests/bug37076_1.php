<?php
$xml = simplexml_load_string("<root><foo /></root>");
$xml->{""} .= "bar";
print $xml->asXML();
?>
===DONE===