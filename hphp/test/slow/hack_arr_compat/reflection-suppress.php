<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum BarEnum : string {
  BAR = '1';
  FOO = '1';
}

function foobar(
  $_a = BarEnum::BAR,
  $_b = BarEnum::FOO,
) :mixed{
}
<<__EntryPoint>> function main(): void {
new ReflectionFunction('foobar') |> $$->getParameters();
echo "DONE\n";
}
