<?php <<__EntryPoint>> function main() {
$dom = new domdocument;
$dom->load("compress.zlib://".dirname(__FILE__)."/book.xml.gz");
print $dom->saveXML();
}
