<?hh

<<__Sealed(SomeInterface::class)>>
interface MyInterface { const FOO = 42; }

<<__EntryPoint>>
function main_sealed_classes13() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'someinterface2' => 'sealed_classes13.inc',
      ],
    ],
    __DIR__.'/',
  );
  var_dump(SomeInterface2::FOO);
}
