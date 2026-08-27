<?hh
<<__EntryPoint>> function main(): void {
$doc = new DOMDocument();
$doc->loadXML(
'<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE root [
    <!ENTITY test SYSTEM "data:text/plain;base64,aGVsbG8gd29ybGQ=">
]>
<root>&test;</root>',
LIBXML_DTDLOAD | LIBXML_NOENT
);

$text = 'hello world';
// libxml2 2.13+ includes entity declarations in document text content.
$expected = LIBXML_VERSION >= 21300 ? $text.$text : $text;
var_dump($doc->textContent === $expected);
}
