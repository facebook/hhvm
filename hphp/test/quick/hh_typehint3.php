<?hh

class :url {
}

function foo(?:url $xhp_object): void {
}
<<__EntryPoint>> function main(): void {
foo(<url />);
foo(null);
foo(vec[1,2,3]);
}
