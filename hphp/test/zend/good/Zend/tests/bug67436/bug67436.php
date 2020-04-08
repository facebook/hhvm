<?hh
<<__EntryPoint>> function main(): void {
HH\autoload_set_paths(
	dict[
		'class' => dict[
			'a' => 'a.php',
			'b' => 'b.php',
			'c' => 'c.php',
		],
	],
	__DIR__.'/',
);

set_error_handler(function ($errno, $errstr, $errfile, $errline) {
}, error_reporting());

a::staticTest();

$b = new b();
$b->test();
}
