<?hh <<__EntryPoint>> function main(): void {
$arr = darray [b"foo\0bar" => b"foo\0bar"];
foreach ($arr as $key => $_) {}
echo strlen($key), ': ';
echo urlencode($key), "\n";
}
