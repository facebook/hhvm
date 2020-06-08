<?hh

namespace {
<<__EntryPoint>>
function main() {
  $map = darray['function' => darray[], 'failure' => function ($kind, $name) {
    echo "Autoload $kind $name\n";
  }];
  HH\autoload_set_paths($map, '');

  \foo\test();
}
}

namespace foo {}
