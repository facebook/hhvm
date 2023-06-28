<?hh

// Warning: line numbers are sensitive, do not change
abstract final class DebuggerBreak5 { public static $x; }

function bad() :mixed{
  DebuggerBreak5::$x += 10;
  return true;
}
<<__EntryPoint>> function main() :mixed{
DebuggerBreak5::$x = 1;
echo DebuggerBreak5::$x."\n";
echo DebuggerBreak5::$x."\n";
DebuggerBreak5::$x = 1;
}
