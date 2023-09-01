<?hh

function get(): void { var_dump(HH\get_product_attribution_id()); }

function set_and_get($b): void {
  if ($b) {
    HH\set_product_attribution_id(20);
  }
  get();
}

<<__EntryPoint>>
function main(): void {
  HH\set_product_attribution_id(10);
  get(); // 10
  // conditional, should get 10 from here
  set_and_get(false); // 10
  set_and_get(true); // 20
}
