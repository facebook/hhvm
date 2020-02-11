<?hh <<__EntryPoint>> function main(): void {
$category = varray["f"];
$tree = varray[$category];

$iterator = new RecursiveIteratorIterator(
    new RecursiveArrayIterator($tree),
    RecursiveIteratorIterator::SELF_FIRST
);
foreach($iterator as $file);
echo "ok\n";
}
