<?php

function f() {
    $a[] = new E; // error2038 in hack, but legal in php
}
