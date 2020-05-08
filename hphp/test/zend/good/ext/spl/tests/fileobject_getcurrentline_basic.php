<?hh
//line 2
//line 3
//line 4
//line 5
<<__EntryPoint>> function main(): void {
$s = new SplFileObject(__FILE__);
$s->seek(1);
echo $s->getCurrentLine();
echo $s->getCurrentLine();
}
