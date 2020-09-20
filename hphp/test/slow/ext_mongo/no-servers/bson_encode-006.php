<?hh
// BSON document: length
<<__EntryPoint>> function main(): void {
$expected = pack('V', 23);
// element: UTF-8 string
$expected .= pack('Ca*xVa*x', 2, '0', 7, 'foobar');

// element: boolean
$expected .= pack('Ca*xC', 8, '1', 1);

// BSON document: end
$expected .= pack('x');

var_dump($expected === bson_encode(varray['foobar', true]));
}
