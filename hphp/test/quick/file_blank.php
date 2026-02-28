<?hh
<<__EntryPoint>> function main(): void {
  $fname = sys_get_temp_dir().'/'.'blank';
  touch($fname);
  var_dump(file($fname));
  unlink($fname);
}
