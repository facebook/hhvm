<?hh <<__EntryPoint>> function main(): void {
$a = new SimpleXMLElement("<php>testfest</php>");
$a->addAttribute( "", "" );
echo $a->asXML();
}
