<?hh
<<__EntryPoint>> function main(): void {
  $fname = getenv('HPHP_TEST_TMPDIR') . 'blank';
  touch($fname);
  var_dump(file($fname));
  unlink($fname);
}
