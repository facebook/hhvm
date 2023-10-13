<?hh
class B {}
class C extends B {}

function foo1(varray<B> $x): void {}
function test1(varray<C> $x): void { foo1($x); }

function foo2(darray<mixed, B> $x): void {}
function test2(darray<int, C> $x): void { foo2($x); }

function foo3(Traversable<B> $x): void {}
function test3(varray<C> $x): void { foo3($x); }

function foo4(KeyedTraversable<mixed, B> $x): void {}
function test4(darray<int, C> $x): void { foo4($x); }

function foo5(Container<B> $x): void {}
function test5(varray<C> $x): void { foo5($x); }

function foo6(KeyedContainer<arraykey, B> $x): void {}
function test6(darray<int, C> $x): void { foo6($x); }

function foo7(KeyedContainer<arraykey, B> $x): void {}
function test7(darray<int, C> $x): void { foo7($x); }
