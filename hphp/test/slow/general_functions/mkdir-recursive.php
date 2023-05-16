<?hh

<<__EntryPoint>> function main_mkdir_recursive(): void {
  $dirName = sys_get_temp_dir().'/'.'mkdirRecursive';
  mkdir($dirName, 777);
  var_dump(@mkdir($dirName, 777, true));
  rmdir($dirName);
}
