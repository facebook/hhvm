<?hh

function main() {
  apc_add('key',
          miarray(
            1 => 2,
            -95 => array(),
          )
  );
  apc_add('key',
          5);
  var_dump(apc_fetch('key'));
}

main();
