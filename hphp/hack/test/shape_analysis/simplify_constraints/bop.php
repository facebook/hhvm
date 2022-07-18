<?hh

function test_plus(dict<string, int> $d): void {
    $d['a'] + $d['b'];
    inspect($d);
}

function test_minus(dict<string, int> $d): void {
    $d['a'] - $d['b'];
    inspect($d);
}

function test_star(dict<string, int> $d): void {
    $d['a'] * $d['b'];
    inspect($d);
}

function test_slash(dict<string, int> $d): void {
    $d['a'] / $d['b'];
    inspect($d);
}

function test_eqeq(dict<string, int> $d): void {
    $d['a'] == $d['b'];
    inspect($d);
}

function test_eqeqeq(dict<string, int> $d): void {
    $d['a'] === $d['b'];
    inspect($d);
}

function test_starstar(dict<string, int> $d): void {
    $d['a'] ** $d['b'];
    inspect($d);
}

function test_diff(dict<string, int> $d): void {
    $d['a'] != $d['b'];
    inspect($d);
}

function test_ampamp(dict<string, bool> $d): void {
    $d['a'] && $d['b'];
    inspect($d);
}

function test_barbar(dict<string, bool> $d): void {
    $d['a'] || $d['b'];
    inspect($d);
}

function test_lt(dict<string, int> $d): void {
    $d['a'] < $d['b'];
    inspect($d);
}

function test_lte(dict<string, int> $d): void {
    $d['a'] <= $d['b'];
    inspect($d);
}

function test_gt(dict<string, int> $d): void {
    $d['a'] > $d['b'];
    inspect($d);
}

function test_gte(dict<string, int> $d): void {
    $d['a'] >= $d['b'];
    inspect($d);
}

function test_dot(dict<string, string> $d): void {
    $d['a'] . $d['b'];
    inspect($d);
}

function test_amp(dict<string, int> $d): void {
    $d['a'] & $d['b'];
    inspect($d);
}

function test_bar(dict<string, int> $d): void {
    $d['a'] | $d['b'];
    inspect($d);
}

function test_ltlt(dict<string, int> $d): void {
    $d['a'] << $d['b'];
    inspect($d);
}

function test_gtgt(dict<string, int> $d): void {
    $d['a'] >> $d['b'];
    inspect($d);
}

function test_percent(dict<string, int> $d): void {
    $d['a'] % $d['b'];
    inspect($d);
}

function test_xor(dict<string, int> $d): void {
    $d['a'] ^ $d['b'];
    inspect($d);
}

function test_cmp(dict<string, int> $d): void {
    $d['a'] <=> $d['b'];
    inspect($d);
}

function test_eq1(dict<string, int> $d): void {
    $d['a'] = $d['b'];
}

function test_eq2(dict<string, int> $d): void {
    $d['a'] = 42;
}
