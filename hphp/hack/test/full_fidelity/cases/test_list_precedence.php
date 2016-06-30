<?hh
function f() {
  $a += $b + list ($c = 1);
  $a += $b + list ($c = 1, await $b = 2);
}
