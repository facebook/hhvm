<?hh

<<__EntryPoint>>
function main(): void {
  $s = shape('x' => 3, 'y' => null);
  var_dump(Shapes::at($s, 'x'));
  var_dump(Shapes::at($s, 'y'));
  try { $_ = Shapes::at($s, 'missing'); }
  catch (Exception $e) { echo $e->getMessage()."\n"; }
}
