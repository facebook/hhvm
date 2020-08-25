<?hh // partial

async function no_hint_async() {}

function no_hint_generator() {
  yield 3;
}

async function no_hint_async_generator() {
  yield 3;
}
