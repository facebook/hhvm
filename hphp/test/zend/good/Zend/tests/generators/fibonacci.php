<?hh

function fib() {
    list($a, $b) = [1, 1];
    while (true) {
        yield $b;
        list($a, $b) = [$b, $a + $b];
    }
}
<<__EntryPoint>> function main(): void {
foreach (fib() as $n) {
    if ($n > 1000) break;

    var_dump($n);
}
}
