<?hh
<<__EntryPoint>> function main(): void {
$x = 4;

$lambda1 = function () use ($x) {
	echo "$x\n";
};

$lambda1();
$x++;
$lambda1();

echo "Done\n";
}
