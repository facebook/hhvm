<?php /* $Id$ */
<<__EntryPoint>> function main() {
var_dump((object)1 instanceof stdClass);
var_dump(! (object)1 instanceof Exception);
}
