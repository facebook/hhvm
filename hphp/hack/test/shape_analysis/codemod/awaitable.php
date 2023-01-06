<?hh

async function f(): Awaitable<dict<string, mixed>> {
//                  ^
//                  |
// TODO(T140415114): at column 21 expect `Awaitable<shape()>`
// instead produces `shape()`
  return dict[];
}
