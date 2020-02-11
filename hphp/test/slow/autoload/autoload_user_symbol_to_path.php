<?hh

<<__EntryPoint>>
function autoload_user_symbol_to_path(): void {
  var_dump(HH\autoload_set_paths(
    dict[
      'class' => dict[
        'a' => 'autoload_symbol_to_path.php',
        't' => 'autoload_symbol_to_path.php',
      ],
      'constant' => dict[
        'C' => 'autoload_symbol_to_path.php',
      ],
      'function' => dict[
        'main' => 'autoload_symbol_to_path.php',
      ],
    ],
    __DIR__.'/',
  ));

  var_dump(HH\autoload_type_to_path(
    A::class,
  ));
  var_dump(HH\autoload_type_to_path(
    T::class,
  ));
  var_dump(HH\autoload_type_to_path(
    'notexisting',
  ));
  var_dump(HH\autoload_function_to_path(
    'main',
  ));
  var_dump(HH\autoload_function_to_path(
    'notexisting',
  ));
  var_dump(HH\autoload_constant_to_path(
    'C',
  ));
  var_dump(HH\autoload_constant_to_path(
    'notexisting',
  ));
}
