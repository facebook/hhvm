<?hh

function xrange($start, $end, $step = 1) :AsyncGenerator<mixed,mixed,void>{
    for ($i = $start; $i <= $end; $i += $step) {
        yield $i;
    }
}
<<__EntryPoint>> function main(): void {
foreach (xrange(10, 20, 2) as $i) {
    var_dump($i);
}
}
