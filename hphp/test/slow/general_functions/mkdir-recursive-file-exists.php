<?hh

<<__EntryPoint>> function main_mkdir_recursive_file_exists(): void {
  $dirName = __SystemLib\hphp_test_tmppath('mkdirRecursiveFileExists');
  fopen($dirName, "w");
  var_dump(@mkdir($dirName, 777, true));
  unlink($dirName);
}
