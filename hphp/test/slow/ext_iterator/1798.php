<?hh


<<__EntryPoint>>
function main_1798() :mixed{
$sample_dir = __DIR__.'/../../sample_dir';

$files = vec[];
foreach (new DirectoryIterator($sample_dir.'/') as $file) {
  $files[] = $file;
}
var_dump(count($files));

$dir = new DirectoryIterator($sample_dir.'/');
$files = vec[];
 // order changes per machine
foreach ($dir as $fileinfo) {
  if (!$fileinfo->isDot()) {
    $files[] = $fileinfo->getFilename();
  }
}
asort(inout $files);
var_dump(array_values($files));

$iterator = new DirectoryIterator($sample_dir);
$files = dict[];
 // order changes per machine
foreach ($iterator as $fileinfo) {
  if ($fileinfo->isFile()) {
    $str = "BEGIN:\n";
    $name = $fileinfo->getFilename();
    $str .= $name . "\n";
    $fileinfo->getCTime() . "\n";
    $fileinfo->getBasename() . "\n";
    $fileinfo->getBasename('.cpp') . "\n";
    $fileinfo->getGroup() . "\n";
    $fileinfo->getInode() . "\n";
    $fileinfo->getMTime() . "\n";
    $fileinfo->getOwner() . "\n";
    $fileinfo->getPerms() . "\n";
    $fileinfo->getSize() . "\n";
    $fileinfo->getType() . "\n";
    (string)($fileinfo->isDir()) . "\n";
    (string)($fileinfo->isDot()) . "\n";
    (string)($fileinfo->isExecutable()) . "\n";
    (string)($fileinfo->isLink()) . "\n";
    (string)($fileinfo->isReadable()) . "\n";
    (string)($fileinfo->isWritable()) . "\n";
    $str .= "END\n";
    $files[$name] = $str;
  }
}
ksort(inout $files);
foreach ($files as $str) {
  echo $str;
}

$iterator = new RecursiveDirectoryIterator($sample_dir);
$files = dict[];
 // order changes per machine
foreach ($iterator as $fileinfo) {
  if ($fileinfo->isFile()) {
    $files[$fileinfo->getFilename()] = $fileinfo;
  }
}
ksort(inout $files);
foreach ($files as $name => $fileinfo) {
  echo $fileinfo->getFilename() . "\n";
   $fileinfo->getCTime() . "\n";
   $fileinfo->getBasename() . "\n";
   $fileinfo->getBasename('.cpp') . "\n";
   $fileinfo->getFilename() . "\n";
   $fileinfo->getGroup() . "\n";
   $fileinfo->getInode() . "\n";
   $fileinfo->getMTime() . "\n";
   $fileinfo->getOwner() . "\n";
   $fileinfo->getPerms() . "\n";
   $fileinfo->getSize() . "\n";
   $fileinfo->getType() . "\n";
   (string)($fileinfo->isDir()) . "\n";
   (string)($fileinfo->isExecutable()) . "\n";
   (string)($fileinfo->isLink()) . "\n";
   (string)($fileinfo->isReadable()) . "\n";
   (string)($fileinfo->isWritable()) . "\n";
}
}
