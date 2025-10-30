<?hh

enum E : string {
    A = 'a';
    B = 'b';
    C = 'c';
}

class A {
    public function foo(): dict<string, int> {
        $foo = dict[
            E::A => 1,
            E::B => 2,
            E::C => 3,
        ];
        return dict[
            E::A => 1,
            E::B => 2,
            E::C => 3,
        ];
    }
}
