<?hh

function blobkv_read(string $path, string $key): ?string {
  $reader = new HH\BlobKV\Reader($path);
  return $reader->read($key);
}

function blobkv_write(string $path, string $key, string $value): void {
  $writer = new HH\BlobKV\Writer($path);
  $writer->write($key, $value);
  $writer->write($key, $value);
  $writer->finalize();
}
