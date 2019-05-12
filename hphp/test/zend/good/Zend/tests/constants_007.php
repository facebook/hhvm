<?php

const a = 'a';
const A = 'b';


class a {
    const a = 'c';
    const A = 'd';
}
<<__EntryPoint>> function main() {
var_dump(a, A, a::a, a::A);
}
