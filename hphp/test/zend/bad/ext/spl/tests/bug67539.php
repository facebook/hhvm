<?php

$it = new ArrayIterator(array_fill(0,2,'X'), 1 );

function badsort($a, $b) {
        $GLOBALS['it']->unserialize($GLOBALS['it']->serialize());
        return TRUE;
}

$it->uksort('badsort');
