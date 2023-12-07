<?hh
<<__EntryPoint>> function main(): void {
$path = sys_get_temp_dir().'/'.'test.html';

file_put_contents($path, b"foo<br>bar<br>foo");
$fp = fopen($path, "r");
while ($fp && !feof($fp)) {
    echo stream_get_line($fp, 0, "<br>")."\n";
}
fclose($fp);
unlink($path);
}
