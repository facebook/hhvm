<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class Box<T> {}
abstract class Generic<T> {
  abstract const type Item as arraykey;
  public function combined<TM>(named T $c, named Box<TM> $box, named TM $m, named this::Item $i): (T, TM, this::Item) {
    return tuple($c, $m, $i);
  }
}
class IntGeneric<T> extends Generic<T> { const type Item = int; }

function test(IntGeneric<string> $obj, Box<int> $box): void {
  $f = HH\meth_caller(Generic::class, 'combined');
  $f($obj, i='wrong', m='wrong', box=$box, c=1); // Three errors
}
