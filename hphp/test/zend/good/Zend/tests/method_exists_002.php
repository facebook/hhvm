<?hh

class bar {
    static public function stat_a2() :mixed{
    }
    static private function stat_b2() :mixed{
    }
    static protected function stat_c2() :mixed{
    }

    private function method_a() :mixed{
    }
    protected function method_b() :mixed{
    }
    public function method_c() :mixed{
    }
}



class baz extends bar {
    static public function stat_a() :mixed{
    }
    static private function stat_b() :mixed{
    }
    static protected function stat_c() :mixed{
    }

    private function method_a() :mixed{
    }
    protected function method_b() :mixed{
    }
    public function method_c() :mixed{
    }
}
<<__EntryPoint>> function main(): void {
var_dump(method_exists('baz', 'stat_a'));
var_dump(method_exists('baz', 'stat_b'));
var_dump(method_exists('baz', 'stat_c'));
print "----\n";
var_dump(method_exists('baz', 'stat_a2'));
var_dump(method_exists('baz', 'stat_b2'));
var_dump(method_exists('baz', 'stat_c2'));
print "----\n";

$baz = new baz;
var_dump(method_exists($baz, 'method_a'));
var_dump(method_exists($baz, 'method_b'));
var_dump(method_exists($baz, 'method_c'));
print "----\n";
var_dump(method_exists($baz, 'stat_a'));
var_dump(method_exists($baz, 'stat_b'));
var_dump(method_exists($baz, 'stat_c'));
}
