<?hh

function blobkv_delete(string $path, string $key): void {
  $writer = new HH\BlobKV\Writer($path);
  $writer->delete($key);
}
