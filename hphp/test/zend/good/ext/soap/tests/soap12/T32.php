<?hh
<<__EntryPoint>>
function entrypoint_T32(): void {
  \HH\global_set('HTTP_RAW_POST_DATA', <<<EOF
<?xml version='1.0' ?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"> 
  <env:Header>
    <test:requiredHeader xmlns:test="http://example.org/ts-tests"
          env:mustUnderstand="true">foo</test:requiredHeader>
  </env:Header>
  <env:Body>
    <test:echoHeader xmlns:test="http://example.org/ts-tests">
    </test:echoHeader>
  </env:Body>
</env:Envelope>
EOF
);
  include "soap12-test.inc";
  test();
}
