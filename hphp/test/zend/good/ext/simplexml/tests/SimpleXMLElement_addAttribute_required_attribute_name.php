<?php <<__EntryPoint>> function main() {
$a = new SimpleXMLElement("<php>testfest</php>");
$a->addAttribute( "", "" );
echo $a->asXML();
}
