<?hh
//line 2
//line 3
//line 4
<<__EntryPoint>> function main(): void {
$s = new SplFileObject(__FILE__);
$s->seek(12); // seek to line 12
$s->next();
var_dump($s->key());
var_dump($s->valid());
}  // line 11
