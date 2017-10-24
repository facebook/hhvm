<?php

trait T {
    function wat() {
        var_dump(self::class);
    }
}

class F { use T; }
class G { use T; }
class H extends G {}

// TODO: This does not match PHP7. It should output F, G, G.
(new F)->wat();
(new G)->wat();
(new H)->wat();
