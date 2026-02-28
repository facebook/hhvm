<?hh


<<__EntryPoint>>
function main_1799() :mixed{
$dir = new DirectoryIterator(__DIR__.'/../../sample_dir');
$files = vec[];
 // order changes per machine
while($dir->valid()) {
  if(!$dir->isDot()) {
    $files[] = $dir->current()."\n";
  }
  $dir->next();
}
asort(inout $files);
var_dump(array_values($files));
}
