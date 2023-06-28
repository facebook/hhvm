<?hh
class C {
    public static function test() :mixed{
        D::prot();
        print_r(get_class_methods("D"));
    }
}
class D extends C {
    protected static function prot() :mixed{
        echo "Successfully called D::prot().\n";
    }
}
<<__EntryPoint>> function main(): void {
D::test();
}
