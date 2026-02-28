<?hh


function go($a) :mixed{
  if ($a) $x = 5;

  $a[1] = dict[];
  $a[1]['hello'] = 5;
  if (isset($a[2]['hi'])) return true;
  return false;
}
<<__EntryPoint>> function main(): void {
$a = dict[];
var_dump(go($a));
}
