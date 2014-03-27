(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Common
open OUnit

let unittest testdir =
  "hackificator" >::: [
    "regressions files" >:: (fun () ->
      let testdir = Filename.concat testdir "/" in
      let expfiles = Common2.glob (testdir ^ "*.exp") in
      expfiles +> List.iter (fun expfile ->
        let (d,b,_e) = Common2.dbe_of_filename expfile in
        let phpfile = Common2.filename_of_dbe (d,b,"php") in

        (match Hackificator.hackify_thrift phpfile with
        | None ->
          assert_failure (spf "hackificator should at least modify %s" phpfile)
        | Some new_content ->
          let tmpfile = Common.new_temp_file "hackifiy" "php" in
          Common.write_file ~file:tmpfile new_content;
        
          let diff = Common2.unix_diff tmpfile expfile in
          diff +> List.iter pr;
          if List.length diff > 1
          then assert_failure
            (spf "hackificator on %s should have resulted in %s" 
               (Filename.basename phpfile)
               (Filename.basename expfile));
        );
    )
    )
  ]
