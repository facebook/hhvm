<?hh
//line 2
//line 3
//line 4
<<__EntryPoint>> function main(): void {
$s = new SplFileObject(__FILE__);
$s->seek(120);  // seek to line 120
$s->next();
var_dump($s->key());
var_dump($s->valid());
}
