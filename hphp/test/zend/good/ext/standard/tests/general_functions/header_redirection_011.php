<?hh <<__EntryPoint>> function main(): void {
header('HTTP/1.1 305 Use Proxy');
header('Location: http://example.com/');
}
