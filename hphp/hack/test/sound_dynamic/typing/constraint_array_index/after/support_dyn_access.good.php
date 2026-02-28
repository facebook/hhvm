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
): void {
  $y = $s['a'];
}
