<?hh

function normalize_blobkv_error(string $message, string $path): string {
  return str_replace($path, '<path>', $message);
}

function expect_blobkv_exception(
  string $label,
  string $path,
  (function(): void) $operation,
): void {
  try {
    $operation();
    echo $label.": no exception\n";
  } catch (Exception $e) {
    echo $label.': '.get_class($e).': '.
      normalize_blobkv_error($e->getMessage(), $path)."\n";
  }
}

function remove_blobkv_files(string $path): void {
  if (file_exists($path)) {
    unlink($path);
  }
  if (file_exists($path.'.part')) {
    unlink($path.'.part');
  }
  if (file_exists($path.'.lock')) {
    unlink($path.'.lock');
  }
}

function corrupt_blobkv_value_offset(string $path): void {
  $data = file_get_contents($path);
  if ($data === false) {
    throw new Exception('Unable to read '.$path);
  }

  $needle = "\x07bounded\x00\x05";
  $replacement = "\x07bounded\x06\x05";
  $offset = strpos($data, $needle);
  if ($offset === false) {
    throw new Exception('Unable to find BlobKV entry in '.$path);
  }

  $written = file_put_contents(
    $path,
    substr_replace($data, $replacement, $offset, strlen($needle)),
  );
  if ($written === false) {
    throw new Exception('Unable to write '.$path);
  }
}

<<__EntryPoint>>
function main(): void {
  $corrupt_path = sys_get_temp_dir().'/'.uniqid('hhvm_blobkv_corrupt_', true).
    '.hbk';
  remove_blobkv_files($corrupt_path);
  file_put_contents($corrupt_path, 'not a blobkv file');

  expect_blobkv_exception(
    'corrupt reader',
    $corrupt_path,
    () ==> {
      new HH\BlobKV\Reader($corrupt_path);
    },
  );
  unlink($corrupt_path);

  $directory_path = sys_get_temp_dir().'/'.
    uniqid('hhvm_blobkv_finalize_directory_', true).'.hbk';
  remove_blobkv_files($directory_path);
  mkdir($directory_path);

  $writer = new HH\BlobKV\Writer($directory_path);
  $writer->write('key', 'value');

  expect_blobkv_exception(
    'finalize over directory',
    $directory_path,
    () ==> $writer->finalize(),
  );

  expect_blobkv_exception(
    'write after finalize failure',
    $directory_path,
    () ==> $writer->write('again', 'value'),
  );

  expect_blobkv_exception(
    'finalize after finalize failure',
    $directory_path,
    () ==> $writer->finalize(),
  );

  $writer = null;
  var_dump(file_exists($directory_path.'.part'));
  if (file_exists($directory_path.'.part')) {
    unlink($directory_path.'.part');
  }
  rmdir($directory_path);

  $bounds_path = sys_get_temp_dir().'/'.
    uniqid('hhvm_blobkv_corrupt_bounds_', true).'.hbk';
  remove_blobkv_files($bounds_path);
  $writer = new HH\BlobKV\Writer($bounds_path);
  $writer->write('bounded', 'value');
  $writer->finalize();
  corrupt_blobkv_value_offset($bounds_path);

  expect_blobkv_exception(
    'corrupt entry bounds',
    $bounds_path,
    () ==> {
      $reader = new HH\BlobKV\Reader($bounds_path);
      $reader->read('bounded');
    },
  );
  remove_blobkv_files($bounds_path);
}
