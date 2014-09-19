<?php

interface DB {
    public function query($query, string ...$params);
}

class MySQL implements DB {
    public function query($query, string $extraParam = null, string ...$params) { }
}

?>
===DONE===
