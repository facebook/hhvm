<?hh

<<__EntryPoint>> function main(): void {
  $origdir = getcwd();
  $dirname = sys_get_temp_dir().'/'.'hhvm_chdir';
  mkdir($dirname);

  var_dump(chdir($dirname));
  chdir($origdir);
  chmod($dirname, 0600);
  var_dump(chdir($dirname));
  rmdir($dirname);
}
