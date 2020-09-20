<?hh
<<__EntryPoint>> function main(): void {
$path = __SystemLib\hphp_test_tmppath('test.html');

file_put_contents($path, b"foo<br>bar<br>foo");
$fp = fopen($path, "r");
while ($fp && !feof($fp)) {
    echo stream_get_line($fp, 0, "<br>")."\n";
}
fclose($fp);
@unlink($path);
}
