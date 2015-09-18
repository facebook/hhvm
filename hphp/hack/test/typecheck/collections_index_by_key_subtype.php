<?hh // strict

enum IntSubtype : int as int {
  ZERO = 0;
}

enum StringSubtype : string as string {
  FOO = 'foo';
}

interface I {}

abstract class IDerived implements I {}

function arrayTest(array<I> $arr, IntSubtype $key): void {
  echo $arr[$key];
}

function dictTest(array<int, I> $dict, IntSubtype $key): void {
  echo $dict[$key];
}

function mapTest(Map<string, I> $dict, StringSubtype $key): void {
  echo $dict[$key];
}

function vectorTest(Vector<string> $vec, IntSubtype $idx): void {
  echo $vec[$idx];
}

function immutableMapTest(ImmMap<string, I> $map, StringSubtype $key): void {
  echo $map[$key];
}

function indexishTest(Indexish<I, string> $indexish, IDerived $key): void {
  echo $indexish[$key];
}

function keyedContainerTest(
  KeyedContainer<I, string> $container,
  IDerived $key,
): void {
  echo $container[$key];
}
