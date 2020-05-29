<?hh

<<__EntryPoint>> function main(): void {
  $origdir = getcwd();
  $dirname = getenv('HPHP_TEST_TMPDIR') . 'hhvm_chdir';
  mkdir($dirname);

  var_dump(chdir($dirname));
  chdir($origdir);
  chmod($dirname, 0600);
  var_dump(chdir($dirname));
  rmdir($dirname);
}
