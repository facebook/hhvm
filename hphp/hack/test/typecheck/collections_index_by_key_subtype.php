<?hh // strict

enum IntSubtype: int as int {
  ZERO = 0;
}

enum StringSubtype: string as string {
  FOO = 'foo';
}

interface I {}

abstract class IDerived implements I {}

function arrayTest(varray<I> $arr, IntSubtype $key): void {
  $arr[$key];
}

function dictTest(darray<int, I> $dict, IntSubtype $key): void {
  $dict[$key];
}

function mapTest(Map<string, I> $dict, StringSubtype $key): void {
  $dict[$key];
}

function vectorTest(Vector<string> $vec, IntSubtype $idx): void {
  $vec[$idx];
}

function immutableMapTest(ImmMap<string, I> $map, StringSubtype $key): void {
  $map[$key];
}

function keyedContainerTest(
  KeyedContainer<string, string> $container,
  StringSubtype $key,
): void {
  $container[$key];
}
