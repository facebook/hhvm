<?php
while (@$i++<10) {
    var_dump(new class($i) {
        public function __construct($i) {
            $this->i = $i;
        }
    });
}
