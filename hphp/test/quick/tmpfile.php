<?hh
<<__EntryPoint>>
function entrypoint_tmpfile(): void {
  $f = tmpfile();
  if ($f === false)
      exit();

  $path = stream_get_meta_data($f)['uri'];
  var_dump($path);
  var_dump(file_exists($path));
  __hhvm_intrinsics\launder_value($f);
}
