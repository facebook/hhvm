<?hh
async function f1() :Awaitable<mixed>{}
async function f2(): Awaitable<void> {}
async function f3(): AsyncIterator<int> { yield 1; }
function f4() :mixed{ async function () {}; }
function f5() :mixed{ async function (): Awaitable<void> {}; }
function f6() :mixed{ async function (): AsyncIterator<int> { yield 1; }; }
function f7() :mixed{ async () ==> {}; }
function f8() :mixed{ async (): Awaitable<void> ==> {}; }
function f9() :mixed{ async (): AsyncIterator<int> ==> { yield 1; }; }
class C {
  async function f1() :Awaitable<mixed>{}
  async function f2(): Awaitable<void> {}
  async function f3(): AsyncIterator<int> { yield 1; }
  function f4() :mixed{ async function () {}; }
  function f5() :mixed{ async function (): Awaitable<void> {}; }
  function f6() :mixed{ async function (): AsyncIterator<int> { yield 1; }; }
  function f7() :mixed{ async () ==> {}; }
  function f8() :mixed{ async (): Awaitable<void> ==> {}; }
  function f9() :mixed{ async (): AsyncIterator<int> ==> { yield 1; }; }
}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
