<?hh

<<__EntryPoint>>
function main_closed_file_stream_metadata() :mixed{
  $this_file = fopen(__FILE__, 'r');
  fclose($this_file);
  var_dump(stream_get_meta_data($this_file));
}
