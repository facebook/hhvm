<?php

var_dump(strtr("", "ll", "a"));
var_dump(strtr("hello", "", "a"));
var_dump(strtr("hello", "ll", "a"));
var_dump(strtr("hello", array("" => "a")));
var_dump(strtr("hello", array("ll" => "a")));
