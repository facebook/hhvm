<?php
namespace lone {
    function my_factory() {
        return new class{};
    }
    class Outer {
        public function __construct() {
             var_dump(
                my_factory());
        }
    }
    new Outer();
}
