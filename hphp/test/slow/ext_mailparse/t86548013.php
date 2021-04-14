<?hh

<<__EntryPoint>>
function main() {
  var_dump(mailparse_rfc822_parse_addresses("\x0a\x03\x3a\x0a\x2b\x0c\x0d\x29\x3c\x3c\x3c"));
}
