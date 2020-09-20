<?hh
<<__EntryPoint>>
function entrypoint_autoload(): void {

  var_dump(is_callable(varray['D', 'foo']));
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'd' => 'autoload2.inc',
      ],
    ],
    __DIR__.'/',
  );
  var_dump(is_callable(varray['D', 'foo']));

  var_dump(is_callable(varray['C', 'foo']));
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'c' => 'autoload1.inc',
      ],
    ],
    __DIR__.'/',
  );
  var_dump(is_callable(varray['C', 'foo']));
}
