<?php
namespace {
    interface Iface {
        function method(stdClass $o);
    }
}
namespace ns {
    class Foo implements \Iface {
        function method(stdClass $o) { }
    }
    
    (new Foo)->method(new \stdClass);
}
?>
