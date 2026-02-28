<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__EntryPoint>>
function test_authorization_header_entrypoint(): void {
  var_dump(
    (isset(\HH\global_get('_SERVER')['HTTP_AUTHORIZATION']) ? \HH\global_get('_SERVER')['HTTP_AUTHORIZATION'] : null)
  );
}
