let parse (json : string) =
  let json = Hh_json.json_of_string ~strict:true json in
  let (_, response) =
    Watchman.Testing.transform_asynchronous_get_changes_response
      (Watchman.Testing.get_test_env ())
      (Some json)
  in
  response

let unexpected r =
  failwith (Printf.sprintf "unexpected: %s" (Watchman.show_pushed_changes r))

let scm_state_enter () =
  match
    parse
      {|
  {
    "unilateral": true,
    "subscription": "hh_informant_watcher.2141920",
    "metadata": {
      "partial": false,
      "status": "ok",
      "rev": "625cfce00b4047d9b9bc42b908d60220db34a33e",
      "distance": 0
    },
    "root": "/data/sandcastle/boxes/www",
    "state-enter": "hg.transaction",
    "version": "2021-09-21T02:22:30Z",
    "clock": "c:1632860685:38408:1:604376"
  }
|}
  with
  | Watchman.State_enter ("hg.transaction", _) -> true
  | r -> unexpected r

let scm_state_leave () =
  match
    parse
      {|
{
    "unilateral": true,
    "subscription": "hh_informant_watcher.2141920",
    "metadata": {
      "partial": false,
      "status": "ok",
      "rev": "a9905b1a64246cf93cfd55db5edb01e42ac62c23",
      "merge": false,
      "distance": 1
    },
    "root": "/data/sandcastle/boxes/www",
    "version": "2021-09-21T02:22:30Z",
    "clock": "c:1632860685:38408:1:604590",
    "state-leave": "hg.update"
  }
|}
  with
  | Watchman.State_leave ("hg.update", _) -> true
  | r -> unexpected r

let scm_changed_merge_base () =
  match
    parse
      {|
{
  "unilateral": true,
  "subscription": "hh_informant_watcher.4043055",
  "root": "/data/users/ljw/www",
  "is_fresh_instance": false,
  "since": {
    "scm": {
      "mergebase-with": "master",
      "mergebase": "2c372fd90f859b48f49768ab61dcf5ea17a118c5"
    },
    "clock": "c:1635454750:1728625:1:10438219"
  },
  "version": "2021-10-18T02:41:02Z",
  "clock": {
    "scm": {
      "mergebase-with": "master",
      "mergebase": "e0ac0069ccf1a833ec944f280a3448f1327fb9ac"
    },
    "clock": "c:1635454750:1728625:1:10438520"
  },
  "files": ["a/b.php"]
}
|}
  with
  | Watchman.Changed_merge_base (rev, files, "c:1635454750:1728625:1:10438520")
    when Hg.Rev.equal
           rev
           (Hg.Rev.of_string "e0ac0069ccf1a833ec944f280a3448f1327fb9ac")
         && SSet.mem "/path/to/root/a/b.php" files ->
    true
  | r -> unexpected r

let scm_files_changed () =
  match
    parse
      {|
  {
    "unilateral": true,
    "subscription": "hh_informant_watcher.3860424",
    "root": "/data/users/ljw/www",
    "is_fresh_instance": false,
    "since": {
      "scm": {
        "mergebase-with": "master",
        "mergebase": "a1569f320e2d7d716841e8be937b32010c6d8064"
      },
      "clock": "c:1635454750:1728625:1:10436603"
    },
    "version": "2021-10-18T02:41:02Z",
    "clock": {
      "scm": {
        "mergebase-with": "master",
        "mergebase": "a1569f320e2d7d716841e8be937b32010c6d8064"
      },
      "clock": "c:1635454750:1728625:1:10436613"
    },
    "files": [
      "a/b.php",
      "c.php"
    ]
  }
|}
  with
  | Watchman.Files_changed files when SSet.mem "/path/to/root/c.php" files ->
    true
  | r -> unexpected r

let noscm_state_enter () =
  match
    parse
      {|
{
  "unilateral": true,
  "subscription": "hh_type_check_watcher.4043586",
  "metadata": {
    "partial": false,
    "status": "ok",
    "rev": "2c372fd90f859b48f49768ab61dcf5ea17a118c5",
    "distance": 0
  },
  "root": "/data/users/ljw/www",
  "state-enter": "hg.transaction",
  "version": "2021-10-18T02:41:02Z",
  "clock": "c:1635454750:1728625:1:10438227"
}
|}
  with
  | Watchman.State_enter ("hg.transaction", _) -> true
  | r -> unexpected r

let noscm_state_leave () =
  match
    parse
      {|
{
  "unilateral": true,
  "subscription": "hh_type_check_watcher.4043586",
  "metadata": {
    "partial": false,
    "status": "ok",
    "rev": "2c372fd90f859b48f49768ab61dcf5ea17a118c5",
    "distance": 0
  },
  "root": "/data/users/ljw/www",
  "version": "2021-10-18T02:41:02Z",
  "clock": "c:1635454750:1728625:1:10438230",
  "state-leave": "hg.transaction"
}
|}
  with
  | Watchman.State_leave ("hg.transaction", _) -> true
  | r -> unexpected r

let noscm_changed_merge_base () =
  match
    parse
      {|
{
  "unilateral": true,
  "subscription": "hh_type_check_watcher.4043586",
  "root": "/data/users/ljw/www",
  "is_fresh_instance": false,
  "since": "c:1635454750:1728625:1:10444992",
  "version": "2021-10-18T02:41:02Z",
  "clock": "c:1635454750:1728625:1:10445222",
  "files": [
    "foo/bar.php"
  ]
}
|}
  with
  | Watchman.Files_changed files when SSet.mem "/path/to/root/foo/bar.php" files
    ->
    true
  | r -> unexpected r

let noscm_files_changed () =
  match
    parse
      {|
{
  "unilateral": true,
  "subscription": "hh_type_check_watcher.4043586",
  "root": "/data/users/ljw/www",
  "is_fresh_instance": false,
  "since": "c:1635454750:1728625:1:10450566",
  "version": "2021-10-18T02:41:02Z",
  "clock": "c:1635454750:1728625:1:10450567",
  "files": [
    "glib/chessbot.php"
  ]
}
|}
  with
  | Watchman.Files_changed files
    when SSet.mem "/path/to/root/glib/chessbot.php" files ->
    true
  | r -> unexpected r

let tests =
  [
    ("scm_state_enter", scm_state_enter);
    ("scm_state_leave", scm_state_leave);
    ("scm_changed_merge_base", scm_changed_merge_base);
    ("scm_files_changed", scm_files_changed);
    ("noscm_state_enter", noscm_state_enter);
    ("noscm_state_leave", noscm_state_leave);
    ("noscm_changed_merge_base", noscm_changed_merge_base);
    ("noscm_files_changed", noscm_files_changed);
  ]

let () = Unit_test.run_all tests
