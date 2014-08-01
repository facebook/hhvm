<?php
class Test {
    public function run() {
        $r = $this->my_parse_m();
        var_dump ($r);
        return $r;
    }

    public function my_parse_m() {
        $test = true;
        if ($test === true) {
            $a = 'b';
        } else {
            return false;
        }
//      flush();
        return true;
    }
}

$t = new Test();
var_dump ($t->run());