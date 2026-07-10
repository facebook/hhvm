<?hh 
<<__EntryPoint>> function main(): void {
$xml =<<<EOF
<?xml version="1.0" encoding="ISO-8859-1" ?>
<foo>bar<baz/>bar</foo>
EOF;

$sxe = simplexml_load_string($xml);

var_dump($sxe->__toString());

echo "===DONE===\n";
}
