<?hh

class :url {
}

function foo(<<__Soft>> ?:url $xhp_object): void {
}
<<__EntryPoint>> function main(): void {
foo(<url />);
foo(varray[1,2,3]);
foo(null);
}
