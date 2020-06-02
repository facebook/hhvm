<?hh

<<__EntryPoint>> function main(): void {
  $origdir = getcwd();
  $dirname = __SystemLib\hphp_test_tmppath('hhvm_chdir');
  mkdir($dirname);

  var_dump(chdir($dirname));
  chdir($origdir);
  chmod($dirname, 0600);
  var_dump(chdir($dirname));
  rmdir($dirname);
}
