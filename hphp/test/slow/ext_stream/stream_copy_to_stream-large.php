<?hh

// Create two temporary files.
<<__EntryPoint>>
function main_stream_copy_to_stream_seek() :mixed{
  $file1 = tmpfile();

  // Write > 2GB into the file
  // 0x8000 = 32768 = 2^15
  // 0x10000 = 65536 = 2^16
  for ($i = 0; $i < 0x8000 + 5; ++$i) {
    fwrite($file1, str_repeat('A', 0x10000));
  }

  // Rewind the file resource
  rewind($file1);

  // Confirm how many characters we have beyond 2^31
  echo strlen(stream_get_contents($file1, -1, 0x80000000))."\n";

  // Rewind it again since we had to seek!
  rewind($file1);

  $file2 = tmpfile();
  invariant(
    stream_copy_to_stream($file1, $file2) === ((0x8000 + 5) * 0x10000),
    'Failed to copy stream properly',
  );

  // Show us how many characters we get back beyond 2^31
  echo strlen(stream_get_contents($file2, -1, 0x80000000))."\n";

  // Cleanup.
  fclose($file1);
  fclose($file2);
}
