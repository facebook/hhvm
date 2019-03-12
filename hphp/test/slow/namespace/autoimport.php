<?php

namespace AutoimportTest;

include 'autoimport_defs.inc';

class ContainerExt extends Container {}
class TraversableExt extends Traversable {}

$a = new Container;
\var_dump(\get_class($a));

$a = new ContainerExt;
\var_dump(\get_class($a));

$a = new Traversable;
\var_dump(\get_class($a));

$a = new TraversableExt;
\var_dump(\get_class($a));
