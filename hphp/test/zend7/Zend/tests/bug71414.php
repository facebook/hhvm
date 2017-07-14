<?php
interface InterfaceY {
    public function z(): string;
}

trait TraitY {
    public function z(): string {
    }
}

class X {
    public function z() {
    }
}

class Y extends X implements InterfaceY {
    use TraitY;
}

echo "ok";
