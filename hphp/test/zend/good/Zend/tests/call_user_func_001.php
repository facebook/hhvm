<?hh

namespace testing {
    function foobar($str) {
        \var_dump($str);
    }

    abstract class bar {
        protected function prot($str) {
            print "Shouldn't be called!\n";
        }
    }
    class foo extends bar {
        private function priv($str) {
            print "Shouldn't be called!\n";
        }
    }

    <<__EntryPoint>> function main(): void {
    \call_user_func(__NAMESPACE__ .'\foobar', 'foobar');
    $class =  __NAMESPACE__ .'\foo';
    \call_user_func(varray[new $class, 'priv'], 'foobar');
    \call_user_func(varray[new $class, 'prot'], 'foobar');
    }
}
