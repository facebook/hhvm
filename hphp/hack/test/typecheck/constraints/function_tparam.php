<?hh // strict

interface Constraint<T as num> {}

function foo<T as Constraint<mixed>>(T $c): void {}
