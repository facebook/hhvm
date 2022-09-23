<?hh

<<__EntryPoint>>
function main() {
  HH\autoload_set_paths(
    dict[
      'module' => darray[
        'a' => 'autoload-1.inc',
        'b' => 'autoload-1.inc'
      ],
      'failure' => (...$args) ==> var_dump($args),
    ],
    __DIR__.'/'
  );
  include "module.inc";
  var_dump(module_exists('a')); // true
  var_dump(module_exists('b')); // false
  var_dump(module_exists('foo', false)); // true
  var_dump(module_exists('boo', false)); // false
}
