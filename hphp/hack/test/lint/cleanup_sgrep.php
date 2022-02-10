<?hh

function test($x, $y) {
  $_ = await $x ? Y : null;
  $_ = await $x ? null : $y;
  $_ = await $x ?: null;
  await $x ? $y : null;
  await $x ? null : $y;
  await $x ?: null;
  // Note: invariant is automatically rewritten to invariant_violation
  invariant(false, $y);
}
