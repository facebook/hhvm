<?hh

function array_get_covariant_key_dict(dict<string,int> $xs, int $idx): void {
  $xs[$idx];
}

function array_get_covariant_key_keyset(keyset<string> $xs, int $idx): void {
  $xs[$idx];
}
