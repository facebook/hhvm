<?hh
<<__EntryPoint>>
function entrypoint_T31(): void {
  \HH\global_set('HTTP_RAW_POST_DATA', <<<EOF
<?xml version='1.0' ?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"> 
  <env:Body>
    <test:returnVoid xmlns:test="http://example.org/ts-tests">
    </test:returnVoid>
  </env:Body>
</env:Envelope>
EOF
);
  include "soap12-test.inc";
  test();
}
