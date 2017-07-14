<?php
class Test
{
    public function __get($var)
    {
        static $first = true;
        echo '__get '.$var.PHP_EOL;
        if ($first) {
            $first = false;
            $this->$var;
            $this->{$var.'2'};
            $this->$var;
        }
    }
}

$test = new Test;
$test->test;

?>
