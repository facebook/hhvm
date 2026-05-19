<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

<<__EntryPoint>>
function main(): void {
  $items = vec[
    shape('x' => 1, 'y' => 10),
    shape('x' => 2),
    shape('x' => 3, 'y' => 30),
  ];
  foreach ($items as shape('x' => $x, ?'y' => $y, ...)) {
    echo "x="; var_dump($x);
    echo "y="; var_dump($y);
  }
}
