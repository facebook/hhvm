<?hh

function main() {
  var_dump(dict(\array_slice(dict(range(1, 10)), 3, 2, true)));
  var_dump(dict(array_combine(range(1, 10), range(11, 20))));
}
main();
