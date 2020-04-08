<?hh

<<__Sealed(SomeTrait::class)>>
trait MyTrait { public static function foo(): int {return 1;} }

<<__EntryPoint>>
function main_sealed_classes11() {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'sometrait2' => 'sealed_classes11.inc',
      ],
    ],
    __DIR__.'/',
  );
  var_dump(SomeTrait2::foo());
}
