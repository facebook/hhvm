<?hh
<<__EntryPoint>> function main(): void {
  $fname = sys_get_temp_dir().'/'.'data';
  $w = fopen($fname, "w");
  $r = fopen($fname, "r");
  printf("Read %d bytes\n", strlen(fread($r, 10)));
  fwrite($w, "hello\n");
  fclose($w);
  printf("Read %d bytes\n", strlen(fread($r, 10)));
  unlink($fname);
}
