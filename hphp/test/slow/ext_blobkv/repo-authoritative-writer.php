<?hh

<<__EntryPoint>>
function main(): void {
  $path = sys_get_temp_dir().'/'.
    uniqid('hhvm_blobkv_repo_authoritative_', true).'.hbk';
  if (file_exists($path)) {
    unlink($path);
  }
  if (file_exists($path.'.lock')) {
    unlink($path.'.lock');
  }

  $fixture = base64_decode(
    'SEJLVgABKGY1OTA4MGFhYjc0NzIyM2YzYmVjZTE0NmIxNTQ0NDkwMzkzOTE1YzcAAAAAAAAAAwAAAAAAAAAMAAAAAAAAAA5iYXIAAAAAAAAAAA4AAAAFZW1wdHkDAANmb28AAw==',
  );
  $old_schema_len = ord($fixture[6]);
  file_put_contents(
    $path,
    substr($fixture, 0, 6).
      chr(strlen(HHVM_REPO_SCHEMA)).
      HHVM_REPO_SCHEMA.
      substr($fixture, 7 + $old_schema_len),
  );

  $reader = new HH\BlobKV\Reader($path);
  var_dump($reader->read('foo'));
  var_dump($reader->read('empty'));

  try {
    new HH\BlobKV\Writer($path);
    echo "unexpected writer\n";
  } catch (Exception $e) {
    echo "writer rejected\n";
  }
}
