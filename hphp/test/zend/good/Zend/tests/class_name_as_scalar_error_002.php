<?hh

namespace Foo\Bar {
    class One {
        const Baz = parent::class;
    }
    <<__EntryPoint>> function main(): void {}
}
