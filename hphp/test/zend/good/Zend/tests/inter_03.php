<?hh

interface a {
    const b = 2;
}

interface b extends a {
    const c = self::b;
}
<<__EntryPoint>> function main(): void {
var_dump(b::c, a::b);
}
