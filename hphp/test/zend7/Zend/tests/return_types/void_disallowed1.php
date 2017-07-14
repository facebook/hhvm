<?php

function foo(): void {
    return NULL; // not permitted in a void function
}

// Note the lack of function call: function validated at compile-time
