<?hh

class Fubar
{
  const Empty = 'Fubar';  // Empty not reserved when used as a class constant
}
<<__EntryPoint>> function main() {
echo Fubar::Empty . "\n";
}
