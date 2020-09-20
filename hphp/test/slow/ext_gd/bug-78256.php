<?hh

<<__EntryPoint>>
function main() {
  var_dump(exif_read_data(__DIR__.'/bug-78256.jpeg'));
}
