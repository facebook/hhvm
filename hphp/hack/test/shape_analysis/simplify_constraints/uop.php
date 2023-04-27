<?hh

function test_utild(dict<string, int> $d): void {
    ~$d['a'];
    inspect($d);
}

function test_unot(dict<string, bool> $d): void {
    !$d['a'];
    inspect($d);
}

function test_uplus(dict<string, int> $d): void {
    +$d['a'];
    inspect($d);
}

function test_uminus(dict<string, int> $d): void {
    -$d['a'];
    inspect($d);
}

function test_uincr(dict<string, int> $d): void {
    --$d['a'];
    inspect($d);
}

function test_udecr(dict<string, int> $d): void {
    --$d['a'];
    inspect($d);
}

function test_upincr(dict<string, int> $d): void {
    $d['a']++;
    inspect($d);
}

function test_updecr(dict<string, int> $d): void {
    $d['a']--;
    inspect($d);
}

function test_usilence(dict<string, int> $d): void {
    @$d['a'];
    inspect($d);
}
