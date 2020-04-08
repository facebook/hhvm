<?hh

class B {
}

<<__EntryPoint>>
function main_1531() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'a' => '1531.inc',
      ],
    ],
    __DIR__.'/',
  );
  var_dump(class_parents("A", false));
  var_dump(class_parents("A"));
  var_dump(class_exists("A"));
}
