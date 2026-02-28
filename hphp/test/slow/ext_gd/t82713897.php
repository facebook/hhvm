<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(exif_read_data("data://text/plain;base64,TU0AKgAAAAYBFwAFAAAAAgAAAAYA+//Y///BAAj/4f8BEQAFAAAAAgAAAAYBFwAFAAAAAgAAAAD7Bv/Y///BAAj/AAABSiFaLEYAAgAAAAYAAAAAAAAAAAABAAAAAAAAAAAG", "", false, true));
}
