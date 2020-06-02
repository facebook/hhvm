<?hh

<<__EntryPoint>> function main_mkdir_recursive(): void {
  $dirName = __SystemLib\hphp_test_tmppath('mkdirRecursive');
  mkdir($dirName, 777);
  var_dump(@mkdir($dirName, 777, true));
  rmdir($dirName);
}
