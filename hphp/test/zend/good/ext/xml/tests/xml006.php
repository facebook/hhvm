<?hh
<<__EntryPoint>>
function main_entry(): void {
  printf("%s -> %s\n", urlencode("æ"), urlencode(utf8_encode("æ")));
  printf("%s <- %s\n", urlencode(utf8_decode(urldecode("%C3%A6"))), "%C3%A6");
}
