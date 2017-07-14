<?php

function foo(): void {
    return -1; // not permitted in a void function
}

// Note the lack of function call: function validated at compile-time
