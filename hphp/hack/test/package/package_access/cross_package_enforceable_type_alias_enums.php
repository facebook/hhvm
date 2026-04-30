//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TShape = shape('x' => int);
type TInt = int;

//// bar.php
<?hh
// package pkg1

// Regular `enum` and `enum class`: the former enforces its base type at
// runtime via `Class::setEnumType`; the latter does not (its `setEnumType`
// path is gated on AttrEnum which is skipped for enum classes, and its
// synthesized members never observe the base by name on an enforcement path).

// Regular enum base: HHVM's Class::setEnumType runs at class init and fatals
// if the base alias can't be resolved. Non-class alias base errors in v2.strict.
enum EnumWithAliasBase: TInt { X = 1; }

// Enum class base: AttrEnum-guarded setEnumType is skipped; synthesized method
// signatures use erased generics / MemberOf newtype. Base T is never observed
// by name at runtime, so non-class alias base should NOT error.
enum class EnumClassWithAliasBase: TShape {
  TShape X = shape('x' => 1);
}
