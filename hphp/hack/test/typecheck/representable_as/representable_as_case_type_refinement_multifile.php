//// types.php
<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

newtype UserId as \HH\Runtime\RepresentableAs<int> = int;
newtype Username as \HH\Runtime\RepresentableAs<string> = string;

function user_id_from_int(int $i): UserId {
  return $i;
}

function username_from_string(string $s): Username {
  return $s;
}

//// caller.php
<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function take_user_id(UserId $u): void {}
function take_username(Username $u): void {}
function take_int_repr(\HH\Runtime\RepresentableAs<int> $x): void {}
function take_string_repr(\HH\Runtime\RepresentableAs<string> $x): void {}

// In a file separate from the newtype definitions, the case type's
// variants are opaque. The runtime tags exposed via each variant's
// `as RepresentableAs<T>` bound are nevertheless enough to prove
// disjointness and to refine on `is int` / `is string`.
case type UserKey = UserId | Username;

// `is int` against the case type refines $key to the UserId variant.
// The narrowed value flows both as the variant type itself and as
// RepresentableAs<int> via UserId's bound. Without the RepresentableAs
// special case in typing_refinement, the bound chain dilutes to mixed
// and neither flow holds.
function test_is_int_narrows(UserKey $key): void {
  if ($key is int) {
    take_user_id($key);
    take_int_repr($key);
  }
}

// The negative branch should narrow cleanly to the Username variant.
// Without the case-type bound-meet skip, the partition expands into
// (X & null) | (X & nonnull) and is not usable as Username or as
// RepresentableAs<string>.
function test_else_narrows_to_username(UserKey $key): void {
  if ($key is int) {
    // skip
  } else {
    take_username($key);
    take_string_repr($key);
  }
}

function call_each(): void {
  test_is_int_narrows(user_id_from_int(42));
  test_else_narrows_to_username(username_from_string('alice'));
}
