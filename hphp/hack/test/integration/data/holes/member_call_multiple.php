<?hh

class C {
    private static function f(int $x, vec<int> $y): void {}

    public static function call_multiple(bool $x, vec<bool> $y): void {
        /* HH_FIXME[4110] */
        self::f($x,$y);
    }
}
