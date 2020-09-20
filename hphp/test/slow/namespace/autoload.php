<?hh

namespace A {
  <<__EntryPoint>>
  function main() {

    \HH\autoload_set_paths(
      dict[
        'class' => dict[
          'a' => 'autoload.inc',
        ],
      ],
      __DIR__.'/',
    );

    $a = '\\A';
    new $a;

    echo 'Done';
  }
}
