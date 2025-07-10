<?hh
type TShape = supportdyn<shape(
  'a' => supportdyn<shape(
    'b' => string,
    ...
  )>,
  ...
)>;

function test(
  ~TShape $s,
): ~string {
  $y = $s['a'];
  $z = $y['b'];
  return $z;
}
