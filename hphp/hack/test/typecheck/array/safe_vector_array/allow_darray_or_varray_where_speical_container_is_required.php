<?hh

function consumeKeyedTraversable(KeyedTraversable<arraykey, num> $arg): void {}
function consumeKeyedContainer(KeyedContainer<arraykey, num> $arg): void {}
function consumeTraversable(Traversable<num> $arg): void {}
function consumeContainer(Container<num> $arg): void {}

function provideDarrayOrVarrayOfInt(): varray_or_darray<int> {
  return dict["a" => 0];
}

function test(): void {
  consumeKeyedTraversable(provideDarrayOrVarrayOfInt());
  consumeKeyedContainer(provideDarrayOrVarrayOfInt());
  consumeTraversable(provideDarrayOrVarrayOfInt());
  consumeContainer(provideDarrayOrVarrayOfInt());
}
