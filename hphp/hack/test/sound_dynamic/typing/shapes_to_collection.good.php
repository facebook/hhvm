<?hh

function test1(shape('a' => int, 'b' => float) $s1, supportdyn<shape('a' => int, 'b' => float, ...)> $s2):void {
  $df = Shapes::toDict<>;
  $af = Shapes::toArray<>;
  $d1 = Shapes::toDict($s1);
  $d2 = Shapes::toDict($s2);
  $d3 = ($df)($s2);
  $a1 = Shapes::toArray($s1);
  $a2 = Shapes::toArray($s2);
  $a3 = ($af)($s2);
  hh_expect_equivalent<dict<string,num>>($d1);
  hh_expect_equivalent<dict<arraykey,supportdyn<mixed>>>($d2);
  // Signature in hhi doesn't include supportdyn unless pessimised
  hh_expect_equivalent<dict<arraykey,mixed>>($d3);
  hh_expect_equivalent<darray<string,num>>($a1);
  hh_expect_equivalent<darray<arraykey,supportdyn<mixed>>>($a2);
  // Signature in hhi doesn't include supportdyn unless pessimised
  hh_expect_equivalent<darray<arraykey,mixed>>($a3);
}

function test2<T1 as shape('a' => int, 'b' => float), T2 as supportdyn<shape('a' => int, 'b' => float, ...)>>(T1 $s1, T2 $s2):void {
  $d1 = Shapes::toDict($s1);
  $d2 = Shapes::toDict($s2);
  $a1 = Shapes::toArray($s1);
  $a2 = Shapes::toArray($s2);
  hh_expect_equivalent<dict<string,num>>($d1);
  hh_expect_equivalent<dict<arraykey,supportdyn<mixed>>>($d2);
  hh_expect_equivalent<darray<string,num>>($a1);
  hh_expect_equivalent<darray<arraykey,supportdyn<mixed>>>($a2);
}
