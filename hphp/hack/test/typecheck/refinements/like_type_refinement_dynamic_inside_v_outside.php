<?hh

function takes_keyed_container(KeyedContainer<arraykey, mixed> $_): void {}

function my_repo(HH\Lib\Ref<null> $null_ref): void {
  $dyn_null = $null_ref->value;
  if ($dyn_null is KeyedTraversable<_, _>) {
    foreach ($dyn_null as $_key => $value) {
      if ($value is KeyedContainer<_, _>) {
        takes_keyed_container($value);
      }
    }
  }
}
