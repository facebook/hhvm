<?hh
<<__EntryPoint>>
function entrypoint_autoload_002(): void {

  HH\autoload_set_paths(
  	dict[
  		'class' => dict[
  			'autoload_root' => 'autoload_root.p5c',
  		],
  	],
  	__DIR__.'/',
  );

  var_dump(get_class_methods('autoload_root'));

  echo "===DONE===\n";
}
