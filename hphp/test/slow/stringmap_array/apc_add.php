<?hh

function main() {
  apc_add('key',
          msarray(
            'one' => 2,
            'sup' => array(),
          )
  );
  apc_add('key',
          5);
  var_dump(apc_fetch('key'));
}

main();
