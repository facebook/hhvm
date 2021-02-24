<?hh
<<__EntryPoint>>
function main(): void {
  $r = fopen("data:,abcde", "r");
  fseek($r, -20, SEEK_CUR);
  var_dump(bin2hex(fread($r, 30)));
}
