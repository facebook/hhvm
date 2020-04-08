<?hh

HH\autoload_set_paths(
  dict[
    'class' => dict[
      'testa' => 'autoload_case1.inc',
    ],
  ],
  __DIR__.'/',
);

TestA::$D = 1;
var_dump(TestA::$D);
