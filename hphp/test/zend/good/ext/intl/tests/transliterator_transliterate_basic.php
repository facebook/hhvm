<?hh
<<__EntryPoint>>
function main_entry(): void {
  $t = transliterator_create("Latin; Title");
  $s = "\xce\x9a\xce\xbf\xce\xbd\xcf\x84\xce\xbf\xce\xb3\xce\xb9\xce\xb1\xce\xbd\xce\xbd\xce\xac\xcf\x84\xce\xbf\xcf\x82, \xce\x92\xce\xb1\xcf\x83\xce\xaf\xce\xbb\xce\xb7\xcf\x82";
  echo $t->transliterate($s),"\n";
  echo transliterator_transliterate($t, $s),"\n";
  echo $t->transliterate($s, 3),"\n";
  echo $t->transliterate($s, 3, 4),"\n";

  echo "Done.\n";
}
