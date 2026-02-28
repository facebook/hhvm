<?hh

interface ia {
    const c = 'Sea';
    const y = 2;
}

class Foo implements ia {
}

class FooBar extends Foo implements ia {
    const x = 1;
    const c = 'Ocean';

    public function show() :mixed{
        return ia::c;
    }
}
<<__EntryPoint>> function main(): void {
new FooBar;
}
