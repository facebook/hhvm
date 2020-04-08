<?hh

<<__EntryPoint>>
function main_1228() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'cat' => '1228.inc',
      ],
    ],
    __DIR__.'/',
  );
  new CaT(1);
  var_dump(class_exists('cat', false));
}
