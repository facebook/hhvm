<?hh

type TS = shape('foo' => int, 'foobar' => float);

class Splat<Targs as (mixed...)> {}
final class Three extends Splat<(int, TS, optional int)> {}
function generic_splat_function<Targs as (mixed...)>(
    Splat<Targs> $splat,
    ... Targs $args,
  ): void {}

function demo(): void {
  generic_splat_function(new Three(), 2, shape('AUTO332'));
}
