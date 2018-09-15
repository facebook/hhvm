<?php

<<__EntryPoint>>
function main_empty_expr_with_parens() {
error_reporting(-1);

var_dump(empty((true || false) && (false || false)));
}
