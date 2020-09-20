<?hh
<<__EntryPoint>> function main(): void {
  $fname = __SystemLib\hphp_test_tmppath('data');
  $w = fopen($fname, "w");
  $r = fopen($fname, "r");
  printf("Read %d bytes\n", strlen(fread($r, 10)));
  fwrite($w, "hello\n");
  fclose($w);
  printf("Read %d bytes\n", strlen(fread($r, 10)));
  unlink($fname);
}
