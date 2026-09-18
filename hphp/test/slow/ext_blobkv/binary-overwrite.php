<?hh

<<__EntryPoint>>
function main(): void {
  $path = sys_get_temp_dir().'/hhvm_blobkv_binary_overwrite_'.
    uniqid('', true).'.hbk';
  if (file_exists($path)) {
    unlink($path);
  }
  if (file_exists($path.'.part')) {
    unlink($path.'.part');
  }
  if (file_exists($path.'.lock')) {
    unlink($path.'.lock');
  }

  $binary = "a\0b\nc";

  $writer = new HH\BlobKV\Writer($path);
  $writer->write('binary', $binary);
  $writer->write('rewrite', 'old');
  $writer->write('rewrite', 'new');
  $writer->finalize();

  $reader = new HH\BlobKV\Reader($path);
  $read_binary = $reader->read('binary');
  var_dump($read_binary === $binary);
  var_dump(strlen($read_binary as nonnull));
  var_dump($reader->read('rewrite'));
}
