<?hh


<<__EntryPoint>>
function main_autoload_set_paths_maps() {
HH\autoload_set_paths(
  Map {
    'class' => Map {
      'b' => 'autoload-class.inc',
    },
  },
  __DIR__.'/'
);

var_dump(new B());
}
