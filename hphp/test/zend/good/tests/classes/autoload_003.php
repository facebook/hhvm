<?hh
<<__EntryPoint>>
function entrypoint_autoload_003(): void {

  HH\autoload_set_paths(
  	dict[
  		'class' => dict[
  			'autoload_derived' => 'autoload_derived.p5c',
  			'autoload_root' => 'autoload_root.p5c',
  		],
  	],
  	__DIR__.'/',
  );

  var_dump(class_exists('autoload_derived'));

  echo "===DONE===\n";
}
