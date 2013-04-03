<?php
 abstract class BaseClass {
        abstract protected function __clone();
    }

    class MommasBoy extends BaseClass {
        protected function __clone() {
            echo __METHOD__, "\n";
        }
    }

    class LatchkeyKid extends BaseClass {
        public function __construct() {
            echo 'In ', __CLASS__, ":\n";
            $kid = new MommasBoy();
            $kid = clone $kid;
        }
        public function __clone() {}
    }

    $obj = new LatchkeyKid();
echo "DONE\n";