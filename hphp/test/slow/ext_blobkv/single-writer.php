<?hh

<<__EntryPoint>>
function main(): void {
  $path = sys_get_temp_dir().'/hhvm_blobkv_single_writer_'.
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
  $stale_part_path = sys_get_temp_dir().'/hhvm_blobkv_stale_part_'.
    uniqid('', true).'.hbk';
  if (file_exists($stale_part_path)) {
    unlink($stale_part_path);
  }
  file_put_contents($stale_part_path.'.part', 'stale writer');

  $writer = new HH\BlobKV\Writer($path);
  $writer->write('first', 'value');

  try {
    new HH\BlobKV\Writer($path);
    echo "unexpected second writer\n";
  } catch (Exception $e) {
    echo "second writer rejected\n";
  }

  try {
    new HH\BlobKV\Writer($stale_part_path);
    echo "unexpected stale part writer\n";
  } catch (Exception $e) {
    echo "stale part rejected\n";
  }

  $writer->finalize();

  $writer2 = new HH\BlobKV\Writer($path);
  $writer2->write('second', 'value');
  $writer2->finalize();

  $reader = new HH\BlobKV\Reader($path);
  var_dump($reader->read('first'));
  var_dump($reader->read('second'));

  unlink($stale_part_path.'.part');
}
