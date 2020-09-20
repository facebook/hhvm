<?hh <<__EntryPoint>> function main(): void {
$s = new SplFileObject(__FILE__);
echo $s->key(), "\n";
$s->fgets();
echo $s->key(), "\n";
$s->fgets();
echo $s->key(), "\n";
$s->fgets();
echo $s->key(), "\n";
$s->rewind();
echo $s->key(), "\n";
$s->fgets();
echo $s->key(), "\n";
$s->fgets();
echo $s->key(), "\n";
$s->fgets();
echo $s->key(), "\n";
}
