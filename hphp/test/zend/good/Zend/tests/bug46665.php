<?hh

<<__EntryPoint>> function main(): void {
HH\autoload_set_paths(
  dict[
    'class' => dict[
      'foo\\bar\\baz' => 'bug46665_autoload.inc',
    ],
  ],
  __DIR__.'/',
);

$baz = '\\Foo\\Bar\\Baz';
new $baz();
echo 'Done';
}
