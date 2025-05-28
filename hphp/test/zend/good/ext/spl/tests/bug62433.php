<?hh <<__EntryPoint>> function main(): void {
$dots = array_keys(dict(new RecursiveDirectoryIterator(__DIR__)));
$ndots = array_keys(dict(new RecursiveDirectoryIterator(__DIR__, FilesystemIterator::SKIP_DOTS)));

var_dump(in_array(__DIR__ . DIRECTORY_SEPARATOR . '.', $dots));
var_dump(in_array(__DIR__ . DIRECTORY_SEPARATOR . '..', $dots));

var_dump(in_array(__DIR__ . DIRECTORY_SEPARATOR . '.', $ndots));
var_dump(in_array(__DIR__ . DIRECTORY_SEPARATOR . '..', $ndots));
}
