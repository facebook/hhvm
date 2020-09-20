<?hh

class Fubar
{
  const Empty = 'Fubar';  // Empty not reserved when used as a class constant
}
<<__EntryPoint>> function main(): void {
echo Fubar::Empty . "\n";
}
