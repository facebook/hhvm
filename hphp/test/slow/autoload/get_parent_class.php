<?hh


<<__EntryPoint>>
function main_get_parent_class() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'c' => 'autoload-class.inc',
        'b' => 'autoload-class.inc',
      ],
    ],
    __DIR__.'/',
  );

  var_dump(get_parent_class('C'));
}
