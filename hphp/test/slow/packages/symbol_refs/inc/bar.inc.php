<?hh

module bar;

type TBar = string;
interface IBar<T> {}

class BarWithGeneric<T> {}
function bar_with_generic<T>(): void {}

class BarWithReifiedGeneric<reify T> {}
function bar_with_reified_generic<reify T>(): void {}
