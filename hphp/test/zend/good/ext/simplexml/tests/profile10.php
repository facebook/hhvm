<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL & ~E_NOTICE);
$root = simplexml_load_string('<?xml version="1.0"?>
<root xmlns:reserved="reserved-ns" xmlns:special="special-ns">
 <child reserved:attribute="Sample" special:attribute="Test" />
</root>
');

$rsattr = $root->child->attributes('reserved-ns');
$spattr = $root->child->attributes('special-ns');

echo $rsattr->offsetGet('attribute');
echo "\n";
echo $spattr->offsetGet('attribute');
echo "\n---Done---\n";
}
