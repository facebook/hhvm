<?hh

<<__Sealed(MyClass::class)>>
class SomeClass { const FOO = 42; }

<<__EntryPoint>>
function main_sealed_classes12() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'class2' => 'sealed_classes12.inc',
      ],
    ],
    __DIR__.'/',
  );
  var_dump(Class2::FOO);
}
