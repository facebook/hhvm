<?hh
<<__EntryPoint>> function main(): void {
$subject = array('1', 'a', '2', 'b', '3', 'A', 'B', '4');
$pattern = array('/\d/', '/[a-z]/', '/[1a]/');
$replace = array('A:$0', 'B:$0', 'C:$0');
$count = -1;

var_dump(preg_filter($pattern, $replace, $subject, -1, inout $count));

echo "===DONE===\n";
}
