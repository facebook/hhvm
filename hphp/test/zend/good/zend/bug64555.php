<?php
 
class Foo {
    protected $unsetme = 1;
    protected $keepme = 2;
     
    public function test() {
        $a = get_object_vars($this);
         
        foreach ($a as $k => $v) {
            if ($k == 'unsetme') {
                echo "Unsetting: $k\n";
                unset($a[$k]);
            } else if ($k == 'keepme') {
                echo "Changing: $k\n";
                $a[$k] = 42;
                $a['keepme'] = 43;
            }
        }
         
        var_dump($a, array_keys($a));
    }
}
 
$f = new Foo;
$f->test();
 
?>