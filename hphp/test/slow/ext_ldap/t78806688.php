<?hh

<<__EntryPoint>>
function main(): void {
  $multiplier = 1431655769;
  $s = str_repeat("a", $multiplier);
  ldap_escape($s);
  echo "FAIL!\n";
}
