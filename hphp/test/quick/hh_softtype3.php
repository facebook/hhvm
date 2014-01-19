<?hh

class :url {
};

function foo(@?:url $xhp_object): void {
}

foo(<url />);
foo(array(1,2,3));
foo(null);
