<?hh
<<__EntryPoint>>
function entrypoint_autoload_001(): void {

  HH\autoload_set_paths(
  	dict[
  		'class' => dict[
  			'autoload_root' => 'autoload_root.p5c',
  		],
  	],
  	__DIR__.'/',
  );

  var_dump(class_exists('autoload_root'));

  echo "===DONE===\n";
}
