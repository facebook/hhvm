<?hh
<<__EntryPoint>>
function entrypoint_T66(): void {
  \HH\global_set('HTTP_RAW_POST_DATA', <<<EOF
<?xml version='1.0' encoding='UTF8'?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"> 
  <env:Header>
    <test:echoOk xmlns:test="http://example.org/ts-tests"
          env:role="http://www.w3.org/2003/05/soap-envelope/role/next">foo</test:echoOk>
  </env:Header>
  <env:Body>
  </env:Body>
</env:Envelope>
EOF
);
  include "soap12-test.inc";
  test();
}
