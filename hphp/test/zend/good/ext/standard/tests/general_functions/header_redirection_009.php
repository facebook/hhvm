<?hh <<__EntryPoint>> function main(): void {
header('HTTP/1.1 303 See Other');
header('Location: http://example.com/');
}
