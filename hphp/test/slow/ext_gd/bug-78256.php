<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(exif_read_data(__DIR__.'/bug-78256.jpeg'));
}
