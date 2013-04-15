<?php

// Test the case that a $GLOBAL set has a W element first---we need to make
// sure the emitter uses a vector instruction instead of trying to do SetS.

function f() { $GLOBALS[] = 1; }
f();
