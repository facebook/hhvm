<?hh
<<__EntryPoint>> function main(): void {
echo "==Mixed==\n";
$a = dict[-1=>'a', '-2'=>'b', 3=>'c', '4'=>'d', 5=>'e', '6001'=>'f', '07'=>'g'];

foreach($a as $k => $v) {
	var_dump($k);
	var_dump($v);
}

echo "==Normal==\n";
$b = vec[];
$b[] = 'a';

foreach($b as $k => $v) {
	var_dump($k);
	var_dump($v);
}

echo "==Negative==\n";
$c = dict['-2' => 'a'];

foreach($c as $k => $v) {
	var_dump($k);
	var_dump($v);
}

echo "==Done==\n";
}
