<?hh

class Yep {}
class Nope {}

function eek(?Nope $_):void {}

function hmm(shape(Nope...) $nopes): void {
   $x = Shapes::idx($nopes, 'x');
   eek($x);
}

function wut(shape(Yep...) $yeps): void {
  hmm($yeps);
}

type UnionTailShape = shape((string | bool)...);
type UnionTailTuple = ((string | bool)...);

function accepts_union_tail_shape<T as UnionTailShape>(T $_): void {}
function accepts_union_tail_tuple<T as UnionTailTuple>(T $_): void {}

function generic_union_tail_consistency(
  shape((string | bool)...) $shape,
  ((string | bool)...) $tuple,
): void {
  accepts_union_tail_shape($shape);
  accepts_union_tail_tuple($tuple);
}
