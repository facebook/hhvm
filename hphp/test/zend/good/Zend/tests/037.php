<?hh

namespace closure;

class closure { static $x = 1;}
<<__EntryPoint>> function main(): void {
$x = __NAMESPACE__;
\var_dump(closure::$x);

\var_dump($x::$x);
}
