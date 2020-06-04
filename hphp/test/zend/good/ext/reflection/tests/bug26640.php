<?hh
<<__EntryPoint>>
function entrypoint_bug26640(): void {

  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'autoload_class' => 'bug26640.inc',
      ],
    ],
    __DIR__.'/',
  );

  $a = new ReflectionClass('autoload_class');

  if (is_object($a)) {
  	echo "OK\n";
  }
}
