<?hh


<<__EntryPoint>>
function main_recursive_iterator_iterator_call() :mixed{
$it = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator(__DIR__.'/../../sample_dir'),
  RecursiveIteratorIterator::SELF_FIRST
);
$files = dict[];
foreach($it as $file) {
  $files[$file->getFilename()] =
    $it->getInnerIterator()->getFilename() == $file->getFilename();
}
ksort(inout $files);
var_dump($files);
}
