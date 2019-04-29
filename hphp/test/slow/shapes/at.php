<?hh

<<__EntryPoint>>
function main(): void {
  $s = shape('x' => 3, 'y' => null);
  var_dump(Shapes::at($s, 'x'));
  var_dump(Shapes::at($s, 'y'));
  $_ = Shapes::at($s, 'missing');
}
