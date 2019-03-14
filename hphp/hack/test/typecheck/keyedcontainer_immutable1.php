<?hh // strict

function test(KeyedContainer<arraykey, mixed> $kc): void {
  $kc[] = 1;
}
