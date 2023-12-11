<?hh

function getFiles($rdi,$depth=0) :mixed{
  if (!is_object($rdi)) return;
  $files = vec[];
  // order changes per machine
  for ($rdi->rewind(); $rdi->valid(); $rdi->next()) {
    if ($rdi->isDot()) continue;
    if ($rdi->isDir() || $rdi->isFile()) {
      $indent = '';
      for ($i = 0; $i<=$depth; ++$i) $indent .= " ";
      $files[] = $indent.$rdi->current()."\n";
      if ($rdi->hasChildren()) {
        $children = $rdi->getChildren();
        getFiles($children, 1+$depth);
      }
    }
  }
  asort(inout $files);
  var_dump(array_values($files));
}

<<__EntryPoint>>
function main_1804() :mixed{
  $rdi = new RecursiveDirectoryIterator(__DIR__.'/../../sample_dir');
  getFiles($rdi);
}
