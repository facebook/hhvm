<?hh

class C {}
function exactness_example(): void {
  $k = keyset[C::class];
  $k[nameof C];
}
