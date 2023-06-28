<?hh


<<__EntryPoint>>
function main_1702() :mixed{
$d = dir(__DIR__."/../../sample_dir/");
echo "Path: " . $d->path . "\n";
$files = varray[];
 // order changes per machine
while (false !== ($entry = $d->read())) {
   $files[] = $entry."\n";
}
asort(inout $files);
var_dump(array_values($files));

$d->rewind();
$files = varray[];
 // order changes per machine
while (false !== ($entry = $d->read())) {
   $files[] = $entry."\n";
}
asort(inout $files);
var_dump(array_values($files));
$d->close();
}
