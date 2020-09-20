<?hh <<__EntryPoint>> function main(): void {
$dom = new domdocument;
$dom->load("compress.zlib://".dirname(__FILE__)."/book.xml.gz");
print $dom->saveXML();
}
