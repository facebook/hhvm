<?php
class AO extends ArrayObject {
}
$o = new AO();
$o['plop'] = $o;

var_dump($o);