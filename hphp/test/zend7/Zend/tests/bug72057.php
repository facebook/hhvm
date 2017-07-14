<?php

set_error_handler(
    function() {
        throw new Exception("My custom error");
    }
);

(function (int $i) { bar(); })("7as");

