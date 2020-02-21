<?hh

HH\autoload_set_paths(
  dict[
    'class' => dict[
      'b' => 'autoload-class.inc',
      'c' => 'autoload-class.inc',
    ],
    'function' => dict[
      'this_is_a_function' => 'autoload-function-and-constant.inc',
    ],
    'constant' => dict[
      'THIS_IS_A_CONSTANT' => 'autoload-function-and-constant.inc',
    ],
  ],
  __DIR__.'/',
);

print "Paths:\n";
$autoload_paths = HH\autoload_get_paths();
\sort(inout $autoload_paths);
foreach ($autoload_paths as $path) {
  print "  $path\n";
}
