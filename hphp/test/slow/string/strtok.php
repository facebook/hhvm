<?php

function tokenize($str, $token) {
    $tokenizedStrArr = array();
    $tokStr = strtok($str, $token);

    while($tokStr !== false) {
        $tokenizedStrArr[] = $tokStr;

        $tokStr = strtok($token);
    }

    return $tokenizedStrArr;
}

var_dump(tokenize('foobarbaz', 'foo'));
var_dump(tokenize('foobarbaz', 'bar'));
var_dump(tokenize('foobarbaz', 'baz'));

var_dump(tokenize('foobarbaz', 'foobar'));
var_dump(tokenize('foobarbaz', 'barbaz'));
var_dump(tokenize('foobarbaz', 'foobaz'));
