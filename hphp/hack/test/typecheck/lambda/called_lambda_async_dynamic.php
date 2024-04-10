<?hh

function d(dynamic $_) : void {}

async function f(): Awaitable<void> {
  d(async {return true;});
}
