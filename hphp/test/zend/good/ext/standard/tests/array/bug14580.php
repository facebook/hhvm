<?hh <<__EntryPoint>> function main(): void {
$arr = dict[b"foo\0bar" => b"foo\0bar"];
foreach ($arr as $key => $_) {}
echo strlen($key), ': ';
echo urlencode($key), "\n";
}
