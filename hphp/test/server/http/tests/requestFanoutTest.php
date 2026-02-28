<?hh

<<__EntryPoint>>
function main(): void {
  require_once('test_base.inc');
  init();

  $custom_args = '-vServer.ThreadCount=1'.
    ' -vServer.RequestFanoutLimit=3'.
    ' -vServer.TrackRequestFanout=true'.
    ' -vServer.EnforceRequestFanout=true'.
    ' -vXbox.ServerInfo.ThreadCount=100'.
    ' -vXbox.ServerInfoMaxQueueLength=1000';

  requestAll(
    vec[
      "request_fanout_enforcement.php?count=1",
      "request_fanout_enforcement.php?count=2",
      "request_fanout_enforcement.php?count=3",
    ],
    $custom_args
  );

}
