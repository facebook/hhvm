<?hh

trait T {
}

<<__EntryPoint>>
function main_1532() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'a' => '1532.inc',
      ],
    ],
    __DIR__.'/',
  );
  var_dump(class_uses("A", false));
  var_dump(class_uses("A"));
  var_dump(class_exists("A"));
}
