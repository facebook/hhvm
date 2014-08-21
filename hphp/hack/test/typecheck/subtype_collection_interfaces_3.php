<?hh // strict
class B {}
class C extends B {}

function foo1(Traversable<B> $x): void {}
function test1(Iterable<C> $x): void { foo1($x); }

function foo2(KeyedTraversable<mixed, B> $x): void {}
function test2(KeyedIterable<int, C> $x): void { foo2($x); }

function foo3(Container<B> $x): void {}
function test3(KeyedContainer<int, C> $x): void { foo3($x); }

function foo4(KeyedContainer<mixed, B> $x): void {}
function test4(Indexish<int, C> $x): void { foo4($x); }

function foo5(Iterable<B> $x): void {}
function test5(Vector<C> $x): void { foo5($x); }

function foo6(KeyedIterable<mixed, B> $x): void {}
function test6(Map<string, C> $x): void { foo6($x); }

function foo7(Indexish<mixed, B> $x): void {}
function test7(Map<string, C> $x): void { foo7($x); }

function foo8(Iterator<B> $x): void {}
function test8(KeyedIterator<int, C> $x): void { foo8($x); }

function foo9(KeyedIterator<mixed, B> $x): void {}
function test9(KeyedIterator<int, C> $x): void { foo9($x); }
