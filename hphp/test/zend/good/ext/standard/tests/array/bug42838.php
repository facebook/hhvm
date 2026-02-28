<?hh

function key_compare_func($a, $b)
:mixed{
    if ($a === $b) {
        return 0;
    }
    return (HH\Lib\Legacy_FIXME\gt($a, $b))? 1:-1;
}
<<__EntryPoint>> function main(): void {
$array1 = dict["a" => "green", "b" => "Brown", 'c' => 'blue', 0 => 'red'];
$array2 = dict["a" => "green", "b" => "Brown", 'c' => 'blue', 0 => 'red'];

$result = array_diff_uassoc($array1, $array2, key_compare_func<>);
print_r($result);
}
