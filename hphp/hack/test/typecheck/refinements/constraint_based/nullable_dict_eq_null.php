<?hh

function nullable_dict_eq_null(?dict<string, mixed> $d): dict<string, nonnull> {
  if ($d === null) {
    return dict[];
  } else {
    return $d;
  }
}
