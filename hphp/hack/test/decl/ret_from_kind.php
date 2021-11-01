<?hh

/* HH_FIXME[4030] */
async function no_hint_async() {}

/* HH_FIXME[4030] */
function no_hint_generator() {
  yield 3;
}

/* HH_FIXME[4030] */
async function no_hint_async_generator() {
  yield 3;
}
