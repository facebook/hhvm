<?php
$a = new SimpleXMLElement("<php>testfest</php>");
$a->addAttribute( "", "" );
echo $a->asXML();
?>