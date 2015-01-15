<?hh // strict
class B {}
class C extends B {}

function foo1(array<Traversable<B>> $x): void {}
function test1(array<Iterable<C>> $x): void { foo1($x); }

function foo2(array<mixed, Traversable<B>> $x): void {}
function test2(array<int, Iterable<C>> $x): void { foo2($x); }

function foo3(Traversable<Traversable<B>> $x): void {}
function test3(array<Iterable<C>> $x): void { foo3($x); }

function foo4(KeyedTraversable<mixed, Traversable<B>> $x): void {}
function test4(array<int, Iterable<C>> $x): void { foo4($x); }

function foo5(Container<Traversable<B>> $x): void {}
function test5(array<Iterable<C>> $x): void { foo5($x); }

function foo6(KeyedContainer<mixed, Traversable<B>> $x): void {}
function test6(array<int, Iterable<C>> $x): void { foo6($x); }

function foo7(KeyedContainer<mixed, Traversable<B>> $x): void {}
function test7(array<int, Iterable<C>> $x): void { foo7($x); }
