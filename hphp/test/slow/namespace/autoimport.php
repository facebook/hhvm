<?hh

namespace AutoimportTest;



class ContainerExt extends Container {}
<<__EntryPoint>> function main(): void {
include 'autoimport_defs.inc';
\var_dump(Container::class);

$a = new ContainerExt;
\var_dump(\get_class($a));
\var_dump(\get_parent_class($a));
}
