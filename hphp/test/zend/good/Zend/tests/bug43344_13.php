<?hh
function f($a=dict[namespace\bar=>0]) :mixed{
  foreach ($a as $k => $v) { return $k; }
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
