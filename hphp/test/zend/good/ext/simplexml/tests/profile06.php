<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL & ~E_NOTICE);
$root = simplexml_load_string('<?xml version="1.0"?>
<root xmlns:reserved="reserved-ns">
 <child reserved:attribute="Sample" />
</root>
');

$attr = $root->child->attributes('reserved-ns');
echo $attr->offsetGet('attribute');
echo "\n---Done---\n";
}
