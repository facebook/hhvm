<?hh <<__EntryPoint>> function main(): void {
header("HTTP/1.1 418 I'm a Teapot");
header('Location: http://example.com/');
}
