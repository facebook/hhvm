<?hh <<__EntryPoint>> function main(): void {
$arr = darray [b"foo\0bar" => b"foo\0bar"];
$key = key($arr);
echo strlen($key), ': ';
echo urlencode($key), "\n";
}
