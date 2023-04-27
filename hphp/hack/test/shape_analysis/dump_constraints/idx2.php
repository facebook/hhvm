<?hh

function idx_ife(dict<string, mixed> $d, bool $b): void {
    if ($b) {
        $d['a'];
        idx($d, 'b');
        $d['c1'];
        idx($d, 'd1');
    } else {
        $d['a'];
        idx($d, 'b');
        $d['c2'];
        idx($d, 'd2');
    }
}
