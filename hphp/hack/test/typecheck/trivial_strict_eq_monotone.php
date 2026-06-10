<?hh
function get_year(): int {
  return 2026;
}

function repro(string $year): bool {
  return $year === get_year();
}

function int_eq_string(int $i, string $s): bool {
  return $i === $s;
}

function string_neq_int(string $s, int $i): bool {
  return $s !== $i;
}
