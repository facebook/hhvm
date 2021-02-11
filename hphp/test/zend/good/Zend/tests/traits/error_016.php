<?hh

trait T {
    const A = "baseline";
}

interface I {
    const B = "baseline";
}

class C implements I{
    use T;
}

<<__EntryPoint>>
function main(): void {
    var_dump(C::B);
    var_dump(C::A);
}
