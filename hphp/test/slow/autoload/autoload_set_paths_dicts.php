<?hh

<<__EntryPoint>>
function main() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'b' => 'autoload-class.inc',
        'c' => 'autoload-class.inc',
      ],
    ],
    __DIR__.'/',
  );
  var_dump(get_class(new C()));
}
