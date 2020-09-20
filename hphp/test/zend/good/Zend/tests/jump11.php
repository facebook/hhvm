<?hh
class foobar {
    public function __construct() {
        switch (1) {
            default:
                goto b;
                a:
                    print "ok!\n";
                    break;
                b:
                    print "ok!\n";
                    goto a;
        }
        print "ok!\n";
    }
}
<<__EntryPoint>> function main(): void {
new foobar;
}
