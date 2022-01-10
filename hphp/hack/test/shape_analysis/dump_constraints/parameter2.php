<?hh

// No constraints should be generated because we only consider dicts with
// strings keys at the moment
function f(dict<arraykey, mixed> $param): void {}
