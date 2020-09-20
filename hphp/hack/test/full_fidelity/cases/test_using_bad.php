<?hh
async function f(bool $b) {
  if ($b) {
    using new C();
  }
}
