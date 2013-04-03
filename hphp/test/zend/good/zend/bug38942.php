<?php
class foo {
    public function foo() {}
}

class bar extends foo {
}
print_r(get_class_methods("bar"));
?>