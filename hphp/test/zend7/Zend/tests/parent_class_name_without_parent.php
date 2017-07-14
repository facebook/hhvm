<?php

trait T {
    public function f() {
        var_dump(parent::class);
    }
}

class C {
    use T;
}

(new C)->f();

?>
