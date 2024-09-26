<?hh

// This actually expects a tuple
function apply_tuple_poly<Targs as (mixed...)>((function(...Targs):void) $f, Targs $params):void {
}

// This expects multiple params, but packed as a vec/tuple a la variadics
// As far as the runtime is concerned, it's variadic
function apply_params_poly<Targs as (mixed...)>((function(...Targs):void) $f, ... Targs $params):void {
}
