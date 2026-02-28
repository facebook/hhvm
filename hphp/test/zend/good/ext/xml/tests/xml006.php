<?hh
<<__EntryPoint>>
function main_entry(): void {
  printf("%s -> %s\n", urlencode("\xe6"), urlencode(utf8_encode("\xe6")));
  printf("%s <- %s\n", urlencode(utf8_decode(urldecode("%C3%A6"))), "%C3%A6");
}
