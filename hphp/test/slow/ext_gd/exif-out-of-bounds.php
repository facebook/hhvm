<?hh

<<__EntryPoint>>
function main_exif_out_of_bounds() {
  var_dump(exif_read_data(__DIR__.'/exif-out-of-bounds.jpeg'));
}
