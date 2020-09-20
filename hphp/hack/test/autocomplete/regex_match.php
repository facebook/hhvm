<?hh // strict

use namespace HH\Lib\Regex;

function match<T as Regex\Match>(Regex\Pattern<T> $_): T {
  // unsafe
}

function main(): void {
  $result = match(re"/foo(?<bar>baz)(?<herp>derp)?/");
  \var_dump($result[AUTO332]);
}
