<?php
        class T
        {
                private $var = array();

                public function add($a)
                {
                        array_push($this->var, $a);
                }

                public function __destruct()
                {
                        print_r($this->var);
                }
        }

        class TT extends T
        {
        }
        $t = new TT();
        $t->add("Hello");
        $t->add("World");
?>