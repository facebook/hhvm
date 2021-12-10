<?hh // strict
class Ref<T> {}
final class MyFoo {
    public function __construct(public readonly Ref<mixed> $x) {}
}

function domyfoo(?MyFoo $f): mixed {
    return $f?->x;
}
