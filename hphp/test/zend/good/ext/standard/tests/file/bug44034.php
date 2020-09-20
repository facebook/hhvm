<?hh
<<__EntryPoint>> function main(): void {
$urls = varray[];
$urls[] = "data://text/plain,foo\r\nbar\r\n";
$urls[] = "data://text/plain,\r\nfoo\r\nbar\r\n";
$urls[] = "data://text/plain,foo\r\nbar";

foreach($urls as $url) {
	echo strtr($url, darray["\r" => "\\r", "\n" => "\\n"]) . "\n";
	var_dump(file($url, FILE_IGNORE_NEW_LINES));
}
}
