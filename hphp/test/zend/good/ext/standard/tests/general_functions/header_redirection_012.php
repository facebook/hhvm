<?hh <<__EntryPoint>> function main(): void {
header('HTTP/1.1 307 Temporary Redirect');
header('Location: http://example.com/');
}
