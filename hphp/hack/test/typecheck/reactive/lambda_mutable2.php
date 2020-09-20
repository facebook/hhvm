<?hh // partial
class Foo {}

<<__Rx>>
function my_test_function(<<__OwnedMutable>> Foo $foo): void {
  $x = <<__Rx, __MutableReturn>>(<<__OwnedMutable>> Foo $a) ==> $a;
  $new_foo = $x(\HH\Rx\move($foo));
}
