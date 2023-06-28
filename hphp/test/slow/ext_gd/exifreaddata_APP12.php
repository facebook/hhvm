<?hh

<<__EntryPoint>>
function main_exifreaddata() :mixed{
  $jpeg = "\xff\xd8\xec\x00\x05\x58\x00\x17";
  $data_stream = "data://text/plain;base64," . base64_encode($jpeg);
  var_dump(exif_read_data($data_stream));
}
