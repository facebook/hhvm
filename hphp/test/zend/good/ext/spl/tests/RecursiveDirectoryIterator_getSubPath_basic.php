<?hh
function rrmdir($dir) :mixed{
    foreach(glob($dir . '/*') as $file) {
        if(is_dir($file)) {
            rrmdir($file);
        } else {
            unlink($file);
        }
    }

    rmdir($dir);
}
<<__EntryPoint>>
function main_entry(): void {
  $depth0 = "depth01";
  $depth1 = 'depth1';
  $depth2 = 'depth2';
  $targetDir = sys_get_temp_dir().'/'.$depth0 . DIRECTORY_SEPARATOR . $depth1 . DIRECTORY_SEPARATOR . $depth2;
  mkdir($targetDir, 0777, true);
  touch($targetDir . DIRECTORY_SEPARATOR . 'getSubPath_test.tmp');
  $iterator = new RecursiveDirectoryIterator(sys_get_temp_dir().'/'.$depth0);
  $it = new RecursiveIteratorIterator($iterator);

  $list = vec[];
  while($it->valid()) {
    $list[] = $it->getInnerIterator()->getSubPath();
    $it->next();
  }
  uasort(inout $list,  HH\Lib\Legacy_FIXME\cmp<>);
  foreach ($list as $item) {
  	echo (string)($item) . "\n";
  }

  rrmdir(sys_get_temp_dir().'/'.$depth0);
}
