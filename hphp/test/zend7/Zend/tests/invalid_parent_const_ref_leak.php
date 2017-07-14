<?php

class A {
    const B = parent::C;
}

try {
    A::B;
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}

?>
