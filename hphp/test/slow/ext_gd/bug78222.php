<?hh

<<__EntryPoint>>
function main() {
  exif_read_data(__DIR__.'/bug78222.jpg', 'THUMBNAIL', false, true);
  echo "did not crash\n";
}
