<?hh

const a = 'a';
const A = 'b';


class a {
    const a = 'c';
    const A = 'd';
}
<<__EntryPoint>> function main(): void {
var_dump(a, A, a::a, a::A);
}
