<?hh
class R<reify T> {}

class C {
    const type T = int;
    public function f(
        R<this> $r,
        R<this::T>$rt
    ): void {}
}
