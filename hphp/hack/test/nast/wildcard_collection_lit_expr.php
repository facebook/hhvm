<?hh

function wildcard_vec(): void {
  $x = vec<_>[];
}

function wildcard_keyset(): void {
  $x = keyset<_>[];
}

function wildcard_dict(): void {
  $x = dict<_, _>[];
}
