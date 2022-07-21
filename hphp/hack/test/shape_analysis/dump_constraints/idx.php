<?hh

function idx_with_and_without_opt(dict<string, mixed> $d): void {
    idx($d, 'x');
    idx($d, 'y', 0);
}
