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
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
