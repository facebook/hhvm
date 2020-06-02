<?hh
<<__EntryPoint>> function main(): void {
  $fname = __SystemLib\hphp_test_tmppath('blank');
  touch($fname);
  var_dump(file($fname));
  unlink($fname);
}
