<?php

function f($foo = 1, $bar) { // missing value is illegal in hack, legal in php
    $a[] = new E; // error2038 in hack, but legal in php
}
