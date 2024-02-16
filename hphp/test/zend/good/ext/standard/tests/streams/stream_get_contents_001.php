<?hh
<<__EntryPoint>> function main(): void {
$tmp = tmpfile();

fwrite($tmp, b"12345");

echo stream_get_contents($tmp, 2, 5), "--\n";
echo stream_get_contents($tmp, 2), "--\n";
echo stream_get_contents($tmp, 2, 3), "--\n";
echo stream_get_contents($tmp, 2, -1), "--\n";
}
