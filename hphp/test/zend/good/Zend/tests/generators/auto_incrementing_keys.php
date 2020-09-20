<?hh

function gen() {
    yield 'foo';
    yield 'bar';
    yield 5 => 'rab';
    yield 'oof';
}
<<__EntryPoint>> function main(): void {
foreach (gen() as $k => $v) {
    echo $k, ' => ', $v, "\n";
}
}
