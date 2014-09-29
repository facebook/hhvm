<?hh
async function f1() {}
async function f2(): Awaitable<void> {}
async function f3(): AsyncIterator<int> { yield 1; }
function f4() { async function () {}; }
function f5() { async function (): Awaitable<void> {}; }
function f6() { async function (): AsyncIterator<int> { yield 1; }; }
function f7() { async () ==> {}; }
function f8() { async (): Awaitable<void> ==> {}; }
function f9() { async (): AsyncIterator<int> ==> { yield 1; }; }
class C {
  async function f1() {}
  async function f2(): Awaitable<void> {}
  async function f3(): AsyncIterator<int> { yield 1; }
  function f4() { async function () {}; }
  function f5() { async function (): Awaitable<void> {}; }
  function f6() { async function (): AsyncIterator<int> { yield 1; }; }
  function f7() { async () ==> {}; }
  function f8() { async (): Awaitable<void> ==> {}; }
  function f9() { async (): AsyncIterator<int> ==> { yield 1; }; }
}
if (false) {
  async function g1() {}
  async function g2(): Awaitable<void> {}
  async function g3(): AsyncIterator<int> { yield 1; }
  function g4() { async function () {}; }
  function g5() { async function (): Awaitable<void> {}; }
  function g6() { async function (): AsyncIterator<int> { yield 1; }; }
  function g7() { async () ==> {}; }
  function g8() { async (): Awaitable<void> ==> {}; }
  function g9() { async (): AsyncIterator<int> ==> { yield 1; }; }
  class D {
    async function g1() {}
    async function g2(): Awaitable<void> {}
    async function g3(): AsyncIterator<int> { yield 1; }
    function g4() { async function () {}; }
    function g5() { async function (): Awaitable<void> {}; }
    function g6() { async function (): AsyncIterator<int> { yield 1; }; }
    function g7() { async () ==> {}; }
    function g8() { async (): Awaitable<void> ==> {}; }
    function g9() { async (): AsyncIterator<int> ==> { yield 1; }; }
  }
}
echo "Done\n";
