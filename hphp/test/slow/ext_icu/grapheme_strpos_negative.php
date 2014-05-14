<?php

var_dump(grapheme_strpos('abaa', 'a', 1));
var_dump(grapheme_strpos('abaa', 'a', 0));
var_dump(grapheme_strpos('abaa', 'a', -1));
var_dump(grapheme_strpos('abaa', 'a', -2));

var_dump(grapheme_strrpos('abaa', 'a', 1));
var_dump(grapheme_strrpos('abaa', 'a', 0));
var_dump(grapheme_strrpos('abaa', 'a', -1));
var_dump(grapheme_strrpos('abaa', 'a', -2));
