<?php

class Test {
    function method(): iterable {
        return [];
    }
}

class TestArray extends Test {
    function method(): array {
        return [];
    }
}

class TestTraversable extends Test {
    function method(): Traversable {
        return new ArrayIterator([]);
    }
}

class TestScalar extends Test {
    function method(): int {
        return 1;
    }
}

?>
