<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

$i = 1;
do
{
    echo "$i\t".($i * $i)."\n"; // output a table of squares
    ++$i;
}
while ($i <= 10);
}
