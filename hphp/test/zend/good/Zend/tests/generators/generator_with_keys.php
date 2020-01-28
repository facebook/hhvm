<?hh

function reverse(array $array) {
    end(inout $array);
    while (null !== $key = key($array)) {
        yield $key => current($array);
        prev(inout $array);
    }
}
<<__EntryPoint>> function main(): void {
$array = darray[
    'foo' => 'bar',
    'bar' => 'foo',
];

foreach (reverse($array) as $key => $value) {
    echo $key, ' => ', $value, "\n";
}
}
