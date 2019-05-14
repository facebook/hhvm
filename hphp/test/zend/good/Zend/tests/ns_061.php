<?php
class A {}
use \A as B;
<<__EntryPoint>> function main() {
echo get_class(new B)."\n";
}
