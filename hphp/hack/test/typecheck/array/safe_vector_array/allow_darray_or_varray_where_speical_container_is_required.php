<?hh // strict

function consumeKeyedTraversable(KeyedTraversable<arraykey, num> $arg): void {}
function consumeKeyedContainer(KeyedContainer<arraykey, num> $arg): void {}
function consumeIndexish(Indexish<arraykey, num> $arg): void {}
function consumeTraversable(Traversable<num> $arg): void {}
function consumeContainer(Container<num> $arg): void {}

function provideDarrayOrVarrayOfInt(): darray_or_varray<int> {
  return darray["a" => 0];
}

function test(): void {
  consumeKeyedTraversable(provideDarrayOrVarrayOfInt());
  consumeKeyedContainer(provideDarrayOrVarrayOfInt());
  consumeIndexish(provideDarrayOrVarrayOfInt());
  consumeTraversable(provideDarrayOrVarrayOfInt());
  consumeContainer(provideDarrayOrVarrayOfInt());
}
