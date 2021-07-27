<?hh <<__EntryPoint>> function main(): void {
$dom = new DOMDocument;
$dom->load("compress.zlib://".dirname(__FILE__)."/book.xml.gz");
print $dom->saveXML();
}
