<?hh

enum MyEnum: string {
  TYPE_A = "A value";
  TYPE_B = "B value";
  TYPE_C = "C value";
}

class Splat<Targs as (mixed...)> {}
final class Three extends Splat<(int, MyEnum, optional int)> {}
function generic_splat_function<Targs as (mixed...)>(
    Splat<Targs> $splat,
    ... Targs $args,
  ): void {}

function demo(): void {
  generic_splat_function(new Three(), 2, AUTO332);
}
