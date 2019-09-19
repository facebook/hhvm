<?hh
<<__EntryPoint>> function main(): void {
$url = "http://www.payp\xD0\xB0l.com";

$issues = 0;
$x = new Spoofchecker();
echo "Checking if words are confusable\n";
var_dump($x->areConfusable("hello, world", "goodbye, world", inout $issues));
var_dump($x->areConfusable("hello, world", "hello, world", inout $issues));
var_dump($x->areConfusable("hello, world", "he11o, wor1d", inout $issues));
}
