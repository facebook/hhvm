<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$arr = darray ["foo\0bar" => "foo\0bar"];
while (list($key, $val) = each(inout $arr)) {
    echo strlen($key), ': ';
    echo urlencode($key), ' => ', urlencode($val), "\n";
}
}
