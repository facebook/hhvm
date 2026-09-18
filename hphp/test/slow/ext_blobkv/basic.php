<?hh

<<__EntryPoint>>
function main(): void {
  $path = sys_get_temp_dir().'/'.uniqid('hhvm_blobkv_basic_', true).'.hbk';
  if (file_exists($path)) {
    unlink($path);
  }
  if (file_exists($path.'.part')) {
    unlink($path.'.part');
  }
  if (file_exists($path.'.lock')) {
    unlink($path.'.lock');
  }

  $old_umask = umask(0);
  try {
    $writer = new HH\BlobKV\Writer($path);
  } finally {
    umask($old_umask);
  }
  $writer->write('alpha', 'one');
  $writer->write('beta', 'two');

  var_dump(file_exists($path));
  var_dump(file_exists($path.'.part'));
  var_dump(file_exists($path.'.lock'));

  $writer->finalize();

  var_dump(file_exists($path));
  var_dump(file_exists($path.'.part'));
  var_dump(file_exists($path.'.lock'));

  $reader = new HH\BlobKV\Reader($path);
  var_dump($reader->read('alpha'));
  var_dump($reader->read('beta'));
  var_dump($reader->read('missing'));
}
