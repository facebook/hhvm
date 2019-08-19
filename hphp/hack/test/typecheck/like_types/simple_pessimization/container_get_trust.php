<?hh

function test(
  shape('a' => int) $s_untrusted,
  vec<int> $v_untrusted,
): void {
  hh_show($s_untrusted['a']);
  hh_show($v_untrusted[0]);

  $s_trusted = shape('a' => 4);
  hh_show($s_trusted['a']);
  $v_trusted = vec[0];
  hh_show($v_trusted[0]);
}
