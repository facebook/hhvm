<?php

var_dump(filter_var("?><!@#$%^&*()}{~Qwertyuilfdsasdfgmnbvcxcvbn", FILTER_SANITIZE_SPECIAL_CHARS));
var_dump(filter_var("<data&sons>", FILTER_SANITIZE_SPECIAL_CHARS));
var_dump(filter_var("", FILTER_SANITIZE_SPECIAL_CHARS));

var_dump(filter_var("?><!@#$%^&*()}{~Qwertyuilfdsasdfgmnbvcxcvbn", FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_ENCODE_LOW));
var_dump(filter_var("<data&sons>", FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_ENCODE_LOW));
var_dump(filter_var("", FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_ENCODE_LOW));

var_dump(filter_var("?><!@#$%^&*()}{~Qwertyuilfdsasdfgmnbvcxcvbn", FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_ENCODE_HIGH));
var_dump(filter_var("<data&sons>", FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_ENCODE_HIGH));
var_dump(filter_var("", FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_ENCODE_HIGH));

var_dump(filter_var("кириллица", FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_ENCODE_HIGH));
var_dump(filter_var("кириллица", FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_ENCODE_LOW));

echo "Done\n";
?>