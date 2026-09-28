<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

function return_dynamic_as_vec(dynamic $value): vec<int> {
  return $value;
}

function return_dynamic_as_vec_mixed(dynamic $value): vec<mixed> {
  return $value;
}

function return_dynamic_as_dict_top(
  dynamic $value,
): dict<arraykey, mixed> {
  return $value;
}

function return_dynamic_as_keyset_top(dynamic $value): keyset<arraykey> {
  return $value;
}

function return_dynamic_as_vec_or_dict_top(
  dynamic $value,
): vec_or_dict<arraykey, mixed> {
  return $value;
}

function return_dynamic_as_generic<<<__Explicit>> T>(dynamic $value): T {
  return $value;
}

enum PartiallyEnforcedEnum : int as int {
  VALUE = 0;
}

function return_dynamic_as_enum(
  dynamic $value,
): PartiallyEnforcedEnum {
  return $value;
}

function return_dynamic_as_partially_enforced_enum(
  dynamic $value,
): (~PartiallyEnforcedEnum & arraykey) {
  return $value;
}

function return_dynamic_as_int(dynamic $value): int {
  return $value;
}

function return_dynamic_as_dynamic(dynamic $value): dynamic {
  return $value;
}

function return_like_as_like(~vec<int> $value): ~vec<int> {
  return $value;
}

async function return_dynamic_as_async_vec(
  dynamic $value,
): Awaitable<vec<int>> {
  return $value;
}

async function return_dynamic_as_async_int(dynamic $value): Awaitable<int> {
  return $value;
}

async function return_dynamic_as_async_dynamic(
  dynamic $value,
): Awaitable<dynamic> {
  return $value;
}

class C {
  public function returnDynamicAsDict(
    dynamic $value,
  ): dict<string, int> {
    return $value;
  }
}

function make_dynamic_returning_closure(): (function(dynamic): vec<int>) {
  return (dynamic $value): vec<int> ==> $value;
}
