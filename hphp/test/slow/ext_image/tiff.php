<?hh

<<__EntryPoint>>
function main_tiff() {
var_dump(exif_read_data(__DIR__ . '/images/gh1193.tiff'));
}
