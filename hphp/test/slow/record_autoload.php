<?hh

function create() : Foo {
  return Foo['x' => 10];
}

function dump(Foo $f) : void {
  var_dump($f['x']);
}

<<__EntryPoint>> function main(): void {
HH\autoload_set_paths(
  darray[
    'function' => darray[
      'funn' => 'autoload_record_foo.inc',
    ],

    'class' => darray[
      'foo' => 'autoload_record_foo.inc',
    ],
  ],
  __DIR__.'/',
);

funn();
$x = create();
dump($x);
}
