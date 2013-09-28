<?php
$xml = new SimpleXMLElement('<foo><bar><![CDATA[]]></bar><baz/></foo>', 
  LIBXML_NOCDATA);
print_r($xml);

$xml = new SimpleXMLElement('<foo><bar></bar><baz/></foo>');
print_r($xml);

$xml = new SimpleXMLElement('<foo><bar/><baz/></foo>');
print_r($xml);
?>
===DONE===