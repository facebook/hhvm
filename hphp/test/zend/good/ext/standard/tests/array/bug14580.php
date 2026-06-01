<?hh <<__EntryPoint>> function main(): void {
$arr = dict["foo\0bar" => "foo\0bar"];
foreach ($arr as $key => $_) {}
echo strlen($key), ': ';
echo urlencode($key), "\n";
}
