<?hh

function idx_with_and_without_opt(dict<string, mixed> $d): void {
    inspect($d);
    idx($d, 'x');
    inspect($d);
    idx($d, 'y', 0);
    inspect($d);
}
