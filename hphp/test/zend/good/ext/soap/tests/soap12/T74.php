<?hh
<<__EntryPoint>>
function entrypoint_T74(): void {
  \HH\global_set('HTTP_RAW_POST_DATA', <<<EOF
<?xml version='1.0' ?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"> 
  <env:Header>
    <test:echoOk xmlns:test="http://example.org/ts-tests"
          env:role="http://www.w3.org/2003/05/soap-envelope/role/next">foo</test:echoOk>
    <test:Unknown xmlns:test="http://example.org/ts-tests">
      <test:raiseFault env:mustUnderstand="1" 
            env:role="http://www.w3.org/2003/05/soap-envelope/role/next">
      </test:raiseFault>
    </test:Unknown>
  </env:Header>
  <env:Body>
  </env:Body>
</env:Envelope>
EOF
);
  include "soap12-test.inc";
  test();
}
