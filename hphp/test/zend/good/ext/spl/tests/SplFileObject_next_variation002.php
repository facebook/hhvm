<?hh
//line 2
//line 3
//line 4
//line 5
<<__EntryPoint>> function main(): void {
$s = new SplFileObject(__FILE__);
$s->seek(2);
echo $s->current();
$s->next();
echo $s->current();

$s->setFlags(SplFileObject::READ_AHEAD);

$s->seek(2);
echo $s->current();
$s->next();
echo $s->current();
}
