<?hh

class Foo {}
class :bar {}
class Super<T> {}

function f<T super Foo>(T $x) :mixed{}
function g<T super :bar>(T $x) :mixed{}
function h<T super Super<T>>(T $x) :mixed{}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
