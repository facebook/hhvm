<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL & ~E_NOTICE);
$root = simplexml_load_string('<?xml version="1.0"?>
<root xmlns:reserved="reserved-ns">
 <child reserved:attribute="Sample" />
</root>
');

$rsattr = $root->child->attributes('reserved');
$myattr = $root->child->attributes('reserved-ns');

echo $rsattr->offsetGet('attribute');
echo $myattr->offsetGet('attribute');
echo "\n---Done---\n";
}
