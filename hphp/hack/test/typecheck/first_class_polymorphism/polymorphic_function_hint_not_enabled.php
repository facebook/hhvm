<?hh

type blah<TOut> = (function<TIn>(TIn): TOut);

function foo((function<T>(T):T) $_): void {}

function bar<Tbar>((function<T as Tbar>(T):T) $_): void {}
