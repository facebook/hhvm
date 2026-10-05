<?hh

class MissingShapeKey {
  const string KEY = 'missing';
}

function dict_key_accesses(dict<string, int> $d): void {
  $_ = $d[MissingShapeKey::KEY];
  $d[MissingShapeKey::KEY] = 1;
}
