<?hh

class C {}

function tparam_upper_bound_where<T>(): void where T as C {}

function tparam_upper_bound<T as C>(): void {}
