<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietarenum E1 : string {
enum E1 : string {
  A = 'A';
}

enum E2 : string {
  use E1;
  B = 'B';
}

function getShape():shape('a' => E2, ?'b' => string) { throw new Exception("A"); }

async function genCheckRules(
): Awaitable<(E1, string)> {
  $s = getShape();
  $x = Shapes::idx($s, 'b') ?? 'a';
  $y = $s['a'] as E1;
  return tuple($y, $x);
}
