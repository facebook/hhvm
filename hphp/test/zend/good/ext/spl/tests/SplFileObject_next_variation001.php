<?hh
//line 2
//line 3
//line 4
<<__EntryPoint>> function main(): void {
$s = new SplFileObject(__FILE__);

$s->seek(13);  // seek to line 13
echo $s->current();
$s->next();
echo $s->current();
var_dump($s->valid());
}  // line 13
