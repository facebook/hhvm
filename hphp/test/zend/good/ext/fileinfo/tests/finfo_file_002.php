<?hh
<<__EntryPoint>> function main(): void {
$fp = finfo_open(FILEINFO_MIME_TYPE);
$results = darray[];

foreach (glob(__DIR__ . "/resources/*") as $filename) {
	if (is_file($filename)) {
		$results["$filename"] = finfo_file($fp, $filename);
	}
}
ksort(inout $results);

var_dump($results);
}
