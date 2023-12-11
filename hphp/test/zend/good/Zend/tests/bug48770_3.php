<?hh

class A {
    public function func($str) :mixed{
        var_dump(__METHOD__ .': '. $str);
    }
    private function func2($str) :mixed{
        var_dump(__METHOD__ .': '. $str);
    }
    protected function func3($str) :mixed{
        var_dump(__METHOD__ .': '. $str);
    }
    private function func22($str) :mixed{
        var_dump(__METHOD__ .': '. $str);
    }
}

class B extends A {
    public function func($str) :mixed{
          self::func2($str);
          self::func3($str);
          call_user_func_array(vec[$this, 'self::inexistent'], vec[$str]);
    }
    private function func2($str) :mixed{
        var_dump(__METHOD__ .': '. $str);
    }
    protected function func3($str) :mixed{
        var_dump(__METHOD__ .': '. $str);
    }
}

class C extends B {
    public function func($str) :mixed{
        parent::func($str);
    }
}
<<__EntryPoint>> function main(): void {
$c = new C;
$c->func('This should work!');
}
