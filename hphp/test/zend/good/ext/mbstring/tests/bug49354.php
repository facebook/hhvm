<?hh
<<__EntryPoint>>
function main_entry(): void {
  $crap = "A\xc3\xa5B\xc3\xa4C\xc3\xb6D\xc3\xbc";
  var_dump(mb_strcut($crap, 0, 100, 'UTF-8'));
  var_dump(mb_strcut($crap, 1, 100, 'UTF-8'));
  var_dump(mb_strcut($crap, 2, 100, 'UTF-8'));
  var_dump(mb_strcut($crap, 3, 100, 'UTF-8'));
  var_dump(mb_strcut($crap, 12, 100, 'UTF-8'));
  var_dump(mb_strcut($crap, 13, 100, 'UTF-8'));
}
