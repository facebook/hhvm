<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Differential / property test for expression-level shape splat.
//
// The emitted `shape(...$p0, ...$p1, ...)` merge is checked against an
// INDEPENDENT reference implementation of rightmost-wins merge (`ref_merge`,
// a plain left-to-right foreach). We enumerate every combination of operand
// contents over a small key space and assert the two agree for every case.
//
// Operands are ordinary dicts (a shape is a dict at runtime); splatting them
// drives the same emitter path as splatting a shape literal. Each part tags its
// values with its own index so a wrong winner is detectable.

const keyset<string> KEYS = keyset['a', 'b', 'c'];

// Independent reference: fold parts left-to-right, later keys overwrite earlier.
function ref_merge(vec<dict<arraykey, mixed>> $parts): dict<arraykey, mixed> {
  $acc = dict[];
  foreach ($parts as $p) {
    foreach ($p as $k => $v) {
      $acc[$k] = $v;
    }
  }
  return $acc;
}

// Enumerate the 2^|KEYS| subsets of keys present in one part, tagging each value
// as "part<idx>:<key>" so distinct parts contribute distinguishable values.
function parts_for(int $idx): vec<dict<arraykey, mixed>> {
  $out = vec[];
  $n = count(KEYS);
  $key_list = vec(KEYS);
  for ($mask = 0; $mask < (1 << $n); $mask++) {
    $d = dict[];
    for ($i = 0; $i < $n; $i++) {
      if (($mask & (1 << $i)) !== 0) {
        $k = $key_list[$i];
        $d[$k] = "part".$idx.":".$k;
      }
    }
    $out[] = $d;
  }
  return $out;
}

function check(string $name, dict<arraykey, mixed> $got, dict<arraykey, mixed> $expected): int {
  if ($got != $expected || array_keys($got) != array_keys($expected)) {
    echo "MISMATCH in $name\n";
    echo "  got:      "; var_dump($got);
    echo "  expected: "; var_dump($expected);
    return 1;
  }
  return 0;
}

<<__EntryPoint>>
function main(): void {
  $fails = 0;
  $cases = 0;

  $ps = parts_for(0);
  $qs = parts_for(1);
  $rs = parts_for(2);

  // Arity 2: shape(...$p, ...$q)
  foreach ($ps as $p) {
    foreach ($qs as $q) {
      $got = shape(...$p, ...$q);
      $exp = ref_merge(vec[$p, $q]);
      $fails += check("arity2", $got, $exp);
      $cases++;
    }
  }

  // Arity 3: shape(...$p, ...$q, ...$r)
  foreach ($ps as $p) {
    foreach ($qs as $q) {
      foreach ($rs as $r) {
        $got = shape(...$p, ...$q, ...$r);
        $exp = ref_merge(vec[$p, $q, $r]);
        $fails += check("arity3", $got, $exp);
        $cases++;
      }
    }
  }

  // Interleaved literal field between two splats:
  //   shape(...$p, 'b' => 'FIELD', ...$q)
  foreach ($ps as $p) {
    foreach ($qs as $q) {
      $got = shape(...$p, 'b' => 'FIELD', ...$q);
      $exp = ref_merge(vec[$p, dict['b' => 'FIELD'], $q]);
      $fails += check("interleaved_field", $got, $exp);
      $cases++;
    }
  }

  // Leading and trailing literal fields around splats:
  //   shape('a' => 'LEAD', ...$p, ...$q, 'c' => 'TRAIL')
  foreach ($ps as $p) {
    foreach ($qs as $q) {
      $got = shape('a' => 'LEAD', ...$p, ...$q, 'c' => 'TRAIL');
      $exp = ref_merge(vec[dict['a' => 'LEAD'], $p, $q, dict['c' => 'TRAIL']]);
      $fails += check("wrapped_fields", $got, $exp);
      $cases++;
    }
  }

  echo "ran $cases cases\n";
  if ($fails === 0) {
    echo "ALL PASSED\n";
  } else {
    echo "$fails FAILURES\n";
  }
}
