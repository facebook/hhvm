<?hh

<<__EntryPoint>> function main(): void {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'a' => 'autoload7-1.inc',
      ],
    ],
    __DIR__.'/',
  );

  $a = '\\A';
  new $a;
  echo 'Done';
}
