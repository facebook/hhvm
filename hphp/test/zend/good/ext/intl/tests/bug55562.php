<?hh
<<__EntryPoint>>
function main_entry(): void {
  var_dump(
    grapheme_substr('FOK', 1, 20), // expected: OK
    grapheme_substr("\xed\x95\x9c\xea\xb5\xad\xec\x96\xb4", 1, 20) //expected: \xea\xb5\xad\xec\x96\xb4
  );
}
