<?php

namespace AutoimportTest;

include 'autoimport_defs.inc';

class ContainerExt extends Container {}
class TraversableExt extends Traversable {}

\var_dump(Container::class);

$a = new ContainerExt;
\var_dump(\get_class($a));
\var_dump(\get_parent_class($a));

\var_dump(Traversable::class);

$a = new TraversableExt;
\var_dump(\get_class($a));
\var_dump(\get_parent_class($a));
