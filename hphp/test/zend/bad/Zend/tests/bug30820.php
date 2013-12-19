<?php
class Blah {
    private static $x;

    public function show() {
        Blah::$x = 1;
        $this->x = 5; // no warning, but refers to different variable

        echo 'Blah::$x = '. Blah::$x ."\n";
        echo '$this->x = '. $this->x ."\n";
    }
}

$b = new Blah();
$b->show();
?>