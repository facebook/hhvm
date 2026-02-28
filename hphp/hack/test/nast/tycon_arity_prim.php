<?hh

function tycon_arity_bool(bool<bool> $x): void {}

function tycon_arity_int(int<int> $x) : void {}

function tycon_arity_float(float<float> $x): void {}

function tycon_arity_num(num<num> $x): void {}

function tycon_arity_string(string<string> $x): void {}

function tycon_arity_arraykey(arraykey<arraykey> $x): void {}

function tycon_arity_resource(resource<resource> $x): void {}

function tycon_arity_mixed(mixed<mixed> $x): void {}

function tycon_arity_nonnull(nonnull<mixed> $x): void {}

function tycon_arity_nothing(nothing<mixed> $x): void {}

function tycon_arity_void(): void<mixed> {}
