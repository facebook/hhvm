<?php

# Make the verify script happy.
print "1..1\n";
print "ok 1\n";

class C extends B {}
class B extends A {}
class A {}
