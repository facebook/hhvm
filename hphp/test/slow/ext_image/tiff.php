<?hh

<<__EntryPoint>>
function main_tiff() :mixed{
var_dump(exif_read_data(__DIR__ . '/images/gh1193.tiff'));
}
