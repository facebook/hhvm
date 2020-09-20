<?hh <<__EntryPoint>> function main(): void {
$s = new SplFileObject(__FILE__);
var_dump($s->isFile());
var_dump($s->isDir());
var_dump($s->isLink());
}
