<?hh <<__EntryPoint>> function main(): void {
header('HTTP/1.1 308 Permanent Redirect');
header('Location: http://example.com/');
}
