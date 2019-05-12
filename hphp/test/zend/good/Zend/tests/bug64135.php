<?php

function exception_error_handler() {
        throw new Exception();
}
<<__EntryPoint>> function main() {
set_error_handler("exception_error_handler");
try {
   $undefined->undefined();
} catch(Exception $e) {
    echo "Exception is thrown";
}
}
