<?hh
type TShape = supportdyn<shape(
  'a' => string,
  ...
)>;

function test(
  ~TShape $s,
): ~string {
  $y = $s['a'];
  return $y;
}
