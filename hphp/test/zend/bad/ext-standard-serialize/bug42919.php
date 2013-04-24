<?php
namespace Foo;
class Bar {
}
echo serialize(new Bar) . "\n";
$x = unserialize(serialize(new Bar));
echo get_class($x) . "\n";
?>