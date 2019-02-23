<?hh // strict
class B {}
class C extends B {}

function foo1(Traversable<B> $x): void {}
function test1(Iterable<C> $x): void { foo1($x); }

function foo2(KeyedTraversable<mixed, B> $x): void {}
function test2(KeyedIterable<mixed, C> $x): void { foo2($x); }

function foo3(Container<B> $x): void {}
function test3(KeyedContainer<int, C> $x): void { foo3($x); }

function foo4(Iterable<B> $x): void {}
function test4(Vector<C> $x): void { foo4($x); }

function foo5(KeyedIterable<string, B> $x): void {}
function test5(Map<string, C> $x): void { foo5($x); }

function foo6(KeyedContainer<arraykey, B> $x): void {}
function test6(Map<string, C> $x): void { foo6($x); }

function foo7(Iterator<B> $x): void {}
function test7(KeyedIterator<int, C> $x): void { foo7($x); }

function foo8(KeyedIterator<mixed, B> $x): void {}
function test8(KeyedIterator<int, C> $x): void { foo8($x); }
