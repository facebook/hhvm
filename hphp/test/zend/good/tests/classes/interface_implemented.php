<?hh

interface if_a {
    function f_a():mixed;
}

interface if_b extends if_a {
    function f_b():mixed;
}

class base {
    function _is_a($sub) :mixed{
        echo 'is_a('.get_class($this).', '.$sub.') = '.(is_a($this, $sub) ? 'yes' : 'no')."\n";
    }
    function test() :mixed{
        echo $this->_is_a('base');
        echo $this->_is_a('derived_a');
        echo $this->_is_a('derived_b');
        echo $this->_is_a('derived_c');
        echo $this->_is_a('derived_d');
        echo $this->_is_a('if_a');
        echo $this->_is_a('if_b');
        echo "\n";
    }
}

class derived_a extends base implements if_a {
    function f_a() :mixed{}
}

class derived_b extends base implements if_a, if_b {
    function f_a() :mixed{}
    function f_b() :mixed{}
}

class derived_c extends derived_a implements if_b {
    function f_b() :mixed{}
}

class derived_d extends derived_c {
}
<<__EntryPoint>> function main(): void {
$t = new base();
$t->test();

$t = new derived_a();
$t->test();

$t = new derived_b();
$t->test();

$t = new derived_c();
$t->test();

$t = new derived_d();
$t->test();
}
