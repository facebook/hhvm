type sql_result = {
  svn_rev : int;
  hg_hash : string;
  everstore_handle : string;
  hh_version : string;
  hhconfig_hash : string;
}

let hack_db_name = "";;
let saved_states_table = ""

let find_nearest ~db ~db_table ~svn_rev ~hh_version ~hhconfig_hash =
  ignore db;
  ignore db_table;
  ignore svn_rev;
  ignore hh_version;
  ignore hhconfig_hash;
  Future.of_value []
