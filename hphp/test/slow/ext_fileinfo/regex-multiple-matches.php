<?hh

function write_magic_file(string $path, string $contents): void {
  $written = file_put_contents($path, $contents);
  invariant(
    $written === strlen($contents),
    'Failed to write the custom magic file',
  );
}

<<__EntryPoint>>
function main(): void {
  $magic_path =
    sys_get_temp_dir().'/hhvm-fileinfo-regex-'.(string)getmypid().'.magic';
  // 'A|BBB' matches twice: 'A' at offset 2 and 'BBB' at offset 4. Only the
  // first match's offset may be applied to the search pointer, as in file(1).
  $magic = "0 regex A|BBB MATCH:%s\n";
  $input = 'XXA_BBBYZ';

  try {
    write_magic_file($magic_path, $magic);
    $finfo = finfo_open(FILEINFO_NONE, $magic_path);
    invariant($finfo !== false, 'Failed to load the custom magic file');

    $result = finfo_buffer($finfo, $input);
    $expected = 'MATCH:A, ASCII text, with no line terminators';
    invariant($result === $expected, 'Unexpected regex match output');

    echo $result."\n";
  } finally {
    if (file_exists($magic_path)) {
      unlink($magic_path);
    }
  }
}
