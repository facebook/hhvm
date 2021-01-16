<?hh
function anons_with_ctx_list(): void {
  $anon_no_use = function()[output]: int use () { return 0; };

  $x = 1;
  $anon_no_ret = function()[non_det] use ($ret) { return $x; };
  $anon = function()[rx]: int use ($ret) { return $x; };
}
