<?hh

function test(shape('k' => vec<int>) $s): void {
  $vec = Shapes::idx($s, 'k', vec[]);
  hh_expect_equivalent<vec<int>>($vec);
}
