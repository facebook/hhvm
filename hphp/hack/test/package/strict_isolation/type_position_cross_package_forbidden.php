//// isolated/c.php
<?hh
// A class in the strict-isolation package `isolated`.
class IsolatedType {}

//// intern/use.php
<?hh
// `intern` does not include `isolated`. Referencing its class in a type position
// is a violation for a strict-isolation package -- including nested positions
// (type arguments) that the rollout carve-outs normally skip.
function takes_isolated(IsolatedType $x): void {}
function returns_isolated(): ?IsolatedType {
  return null;
}
function nested_isolated(): vec<IsolatedType> {
  return vec[];
}
