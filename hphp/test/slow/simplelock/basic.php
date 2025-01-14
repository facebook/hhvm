<?hh

<<__EntryPoint>>
async function main() {
  await HH\SimpleLock\lock("hello");
  await HH\SimpleLock\lock("goodbye");
  HH\SimpleLock\unlock("hello");
  HH\SimpleLock\unlock("bad");
}
