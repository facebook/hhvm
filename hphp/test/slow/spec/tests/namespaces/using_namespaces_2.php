<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

use Graphics\D2, Graphics\D3 as D3;

namespace foo\bar
{
    use my\space\MyClass;
}

namespace another\bar
{
    use my\space\MyClass, xx\xxx as XX, yy\yyy as YY;
    use my\space\AnotherClass;
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
}
