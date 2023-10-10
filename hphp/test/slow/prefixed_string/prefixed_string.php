<?hh

function f(): void {
  $x = re"blah blah\n";
  echo($x);
  $s1 = "Be";
  $s2 = "diff";
  $s = re"{$s1} the {$s2} you want to see.\n";
  echo($s);
}


<<__EntryPoint>>
function main_prefixed_string() :mixed{
f();
}
