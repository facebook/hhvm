<?php
interface I { function foo(); }
abstract class B implements I {}
abstract class C extends B {}
class D extends C {}
echo "Done\n";
