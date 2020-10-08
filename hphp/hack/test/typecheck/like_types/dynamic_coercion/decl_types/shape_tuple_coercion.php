<?hh

final class Hgoldstein<<<__Enforceable>> reify T> {
    public function do(T $val): void {
        $val as T;
    }
}

function hgoldstein_function(dynamic $n): void {
    (new Hgoldstein<shape('a' => int)>())->do($n);
    (new Hgoldstein<(int, string)>())->do($n);
}
