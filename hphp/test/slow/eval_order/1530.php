<?hh

interface I {
}

<<__EntryPoint>>
function main_1530() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'a' => '1530.inc',
      ],
    ],
    __DIR__.'/',
  );
var_dump(class_implements("A", false));
var_dump(class_implements("A"));
var_dump(class_exists("A"));
}
