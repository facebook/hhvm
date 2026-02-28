<?hh


<<__EntryPoint>>
function main_1801() :mixed{
$ite=new RecursiveDirectoryIterator(__DIR__.'/../../sample_dir/');
$bytestotal=0.0;
$nbfiles=0;
$files = vec[];
 // order changes per machine
foreach (new RecursiveIteratorIterator($ite) as $filename=>$cur) {
  if (substr($filename,-1)=='.') continue;
  $filesize=$cur->getSize();
  $bytestotal+=$filesize;
  $nbfiles++;
  $files[] = "$filename => $filesize\n";
}
asort(inout $files);
var_dump(array_values($files));
$bytestotal=number_format($bytestotal);
echo "Total: $nbfiles files, $bytestotal bytes\n";
}
