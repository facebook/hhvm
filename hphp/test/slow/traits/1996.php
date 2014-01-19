<?php

class MY_CLASS {
}
trait MY_TRAIT {
}
interface MY_INTERFACE {
}
var_dump(trait_exists('MY_CLASS'));
var_dump(trait_exists('MY_INTERFACE'));
var_dump(trait_exists('MY_TRAIT'));
var_dump(trait_exists('UNDECLARED'));
var_dump(trait_exists(1));
var_dump(trait_exists(NULL));
