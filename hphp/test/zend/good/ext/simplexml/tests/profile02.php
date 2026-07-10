<?hh <<__EntryPoint>> function main(): void {
$root = simplexml_load_string('<?xml version="1.0"?>
<root>
 <child>Hello</child>
 <child>World</child>
</root>
');

foreach ($root->child as $child) {
    echo $child->__toString()." ";
}
echo "\n---Done---\n";
}
