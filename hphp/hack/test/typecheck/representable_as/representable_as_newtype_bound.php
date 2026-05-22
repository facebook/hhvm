//// types.php
<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

newtype IntId as \HH\Runtime\RepresentableAs<int> = int;
newtype StringId as \HH\Runtime\RepresentableAs<string> = string;

//// caller.php
<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function take_int_repr(\HH\Runtime\RepresentableAs<int> $x): void {}
function take_string_repr(\HH\Runtime\RepresentableAs<string> $x): void {}

// In a file separate from the newtype definitions the newtypes are opaque,
// so only their declared bounds are visible to subtyping. The Tnewtype LHS
// arm of the rule walks the bound chain, so a newtype bounded by
// RepresentableAs<U> flows into a RepresentableAs<U> position.
function test_newtype_int(IntId $val): void {
  take_int_repr($val);
}

function test_newtype_string(StringId $val): void {
  take_string_repr($val);
}
