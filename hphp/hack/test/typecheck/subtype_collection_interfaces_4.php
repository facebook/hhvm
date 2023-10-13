<?hh
class B {}
class C extends B {}

function foo1(Traversable<Traversable<B>> $x): void {}
function test1(Iterable<Iterable<C>> $x): void { foo1($x); }

function foo2(KeyedTraversable<mixed, Traversable<B>> $x): void {}
function test2(KeyedIterable<int, Iterable<C>> $x): void { foo2($x); }

function foo3(Container<Traversable<B>> $x): void {}
function test3(KeyedContainer<int, Iterable<C>> $x): void { foo3($x); }

function foo4(Iterable<Traversable<B>> $x): void {}
function test4(Vector<Iterable<C>> $x): void { foo4($x); }

function foo5(KeyedIterable<string, Traversable<B>> $x): void {}
function test5(Map<string, Iterable<C>> $x): void { foo5($x); }

function foo6(KeyedContainer<arraykey, Traversable<B>> $x): void {}
function test6(Map<string, Iterable<C>> $x): void { foo6($x); }

function foo7(Iterator<Traversable<B>> $x): void {}
function test7(KeyedIterator<int, Iterable<C>> $x): void { foo7($x); }

function foo8(KeyedIterator<mixed, Traversable<B>> $x): void {}
function test8(KeyedIterator<int, Iterable<C>> $x): void { foo8($x); }
