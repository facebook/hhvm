<?hh

class :url {
};

function foo(?:url $xhp_object): void {
}

foo(<url />);
foo(null);
foo(array(1,2,3));
