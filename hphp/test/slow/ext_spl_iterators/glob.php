<?hh


<<__EntryPoint>>
function main_glob() :mixed{
$path = 'glob://' . __DIR__ . '/../../sample_dir/*';

$iters = vec[
  new DirectoryIterator($path),
  new FilesystemIterator($path),
  new GlobIterator($path)
];

foreach ($iters as $iter) {
  var_dump(get_class($iter));
  var_dump($iter->getPath());
  var_dump($iter->getPathname());
  var_dump($iter->getFilename());

  foreach ($iter as $file) {
    echo "$file\n";
  }
}
}
