<?hh
function anons_with_ctx_list(): void {
  $anon_no_use = function()[defaults]: int use () { return 0; };

  $x = 1;
  $anon_no_ret = function()[defaults] use ($ret) { return $x; };
  $anon = function()[defaults]: int use ($ret) { return $x; };
}
