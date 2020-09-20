<?hh
<<__EntryPoint>> function main(): void {
$a = '11';
$b = $a[0];
switch ($b) {
	case '-':
		break;
	default: break;
}

$a = '22';
switch ($a[0]) {
	case '-':
		break;
	default: break;
}

echo "===DONE===\n";
}
