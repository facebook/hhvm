<?php

print "Test begin\n";

$obj = new stdclass;
unset($obj->doh->re->mi->fa->sol->la->ti);
var_dump($obj);

print "Test end\n";
