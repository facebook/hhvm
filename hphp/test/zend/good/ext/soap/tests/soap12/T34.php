<?hh
<<__EntryPoint>>
function entrypoint_T34(): void {
  \HH\global_set('HTTP_RAW_POST_DATA', <<<EOF
<?xml version='1.0' ?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"> 
  <env:Header>
    <test:Unknown xmlns:test="http://example.org/ts-tests" 
          xmlns:env1="http://schemas.xmlsoap.org/soap/envelope/"
          env1:mustUnderstand="true">foo</test:Unknown>
  </env:Header>
  <env:Body>
  </env:Body>
</env:Envelope>
EOF
);
  include "soap12-test.inc";
  test();
}
