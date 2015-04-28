<?hh

class Foo {}
class :bar {}
class Super<T> {}

function f<T super Foo>(T $x) {}
function g<T super :bar>(T $x) {}
function h<T super Super<T>>(T $x) {}
