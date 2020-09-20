<?hh

class C { function __toString() { return 'I'; } }

<<__EntryPoint>> function main(): void {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'i' => 'autoload6-2.inc',
      ],
    ],
    __DIR__.'/',
  );
  var_dump(interface_exists(new C));
}
