<?hh // strict

namespace NS_coalesce;

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015-2016 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function foo(): void {
  echo "executed!", PHP_EOL;
}

function main(): void {
  $arr = ["foo" => "bar", "qux" => null];
  $obj = (object)$arr;

  $a = $arr["foo"] ?? "bang"; // "bar" as $arr["foo"] is set and not null
  $a = $arr["qux"] ?? "bang"; // "bang" as $arr["qux"] is null
  $a = $arr["bing"] ?? "bang"; // "bang" as $arr["bing"] is not set

  $a = $obj->foo ?? "bang"; // "bar" as $obj->foo is set and not null
  $a = $obj->qux ?? "bang"; // "bang" as $obj->qux is null
  $a = $obj->bing ?? "bang"; // "bang" as $obj->bing is not set

  $a = null ?? $arr["bing"] ?? 2; // 2 as null is null, and $arr["bing"] is not set
  var_dump(true ?? foo()); // outputs bool(true), "executed!" does not appear as it short-circuits
}

/* HH_FIXME[1002] call to main in strict*/
main();
