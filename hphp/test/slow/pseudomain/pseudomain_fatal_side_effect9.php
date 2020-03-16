<?hh

<<__Entrypoint>>
function main(): void {
  HH\autoload_set_paths(
    darray[
      'class' => darray[
        'foo' => 'pseudomain_fatal_side_effect9.inc',
      ],
    ],
    __DIR__.'/',
  );

  var_dump((new Foo())->foo());
}
