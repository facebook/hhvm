<?hh
namespace N1 {

    trait T1 {
        public function hello() { return 'hello from t1'; }
    }

    trait T2 {
        public function hello() { return 'hello from t2'; }
    }

}
namespace N2 {
    use N1\T1;
    use N1\T2;
    class A {
        use T1, T2 {
            T1::hello insteadof T2;
            T1::hello as foo;
        }
    }
    <<__EntryPoint>> function main(): void {
    $a = new A;
    echo $a->hello(), \PHP_EOL;
    echo $a->foo(), \PHP_EOL;
    try {
    } catch(namespace \Foo $e)
    {
    }
    }
}
