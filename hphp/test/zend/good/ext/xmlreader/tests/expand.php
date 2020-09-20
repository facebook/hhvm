<?hh 
<<__EntryPoint>> function main(): void {
$basexml = '<?xml version="1.0" encoding="UTF-8"?>
<books><book>base book</book></books>';

$xmlstring = '<?xml version="1.0" encoding="UTF-8"?>
<books><book>new book</book></books>';

$dom = new DOMDocument();
$dom->loadXML($basexml);

$reader = new XMLReader();
$reader->XML($xmlstring);
while ($reader->read()) {
    if ($reader->localName == "book") {
        $node = $reader->expand($dom);
        if ($node->ownerDocument) {
            echo $node->ownerDocument->documentElement->firstChild->textContent . "\n";
        }
        break;
    }
}
$reader->close();
echo "===DONE===\n";
}
