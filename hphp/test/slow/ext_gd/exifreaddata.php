<?hh

<<__EntryPoint>>
function main_exifreaddata() :mixed{
  $jpeg = "\xff\xd8\xc9\x00\x03\x04\x03\xf8";
  $data_stream = "data://text/plain;base64," . base64_encode($jpeg);
  var_dump(exif_read_data($data_stream));
}
