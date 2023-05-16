<?hh

<<__EntryPoint>> function main_mkdir_recursive_file_exists(): void {
  $dirName = sys_get_temp_dir().'/'.'mkdirRecursiveFileExists';
  fopen($dirName, "w");
  var_dump(@mkdir($dirName, 777, true));
  unlink($dirName);
}
