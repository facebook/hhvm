<?hh

function get(): void { var_dump(HH\get_product_attribution_id()); }

function indirection(): void { get(); }

function set_and_get($x): void {
  HH\set_product_attribution_id($x);
  get();
}

<<__EntryPoint>>
function main(): void {
  echo "Not set\n";
  get();

  echo "Set null\n";
  HH\set_product_attribution_id(null);
  get();

  echo "Set 5\n";
  HH\set_product_attribution_id(5);
  get();

  echo "Set lambda 10\n";
  HH\set_product_attribution_id_deferred(()[leak_safe] ==> { return 10; });
  get();

  echo "Set to 15 (get via indirection)\n";
  HH\set_product_attribution_id(15);
  indirection();

  echo "Nested sets 20-25\n";
  HH\set_product_attribution_id(20);
  set_and_get(25);
  get();
}
