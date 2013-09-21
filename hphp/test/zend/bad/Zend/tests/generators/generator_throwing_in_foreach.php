<?php

function gen() {
    throw new Exception("foo");
    yield; // force generator
}

foreach (gen() as $value) { }

?>