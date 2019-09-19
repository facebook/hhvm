type timeouts = {
  package_fetch_timeout: int;
  find_exact_state_timeout: int;
  find_nearest_state_timeout: int;
  current_hg_rev_timeout: int;
  current_base_rev_timeout: int;
}

let default_timeouts =
  {
    package_fetch_timeout = 150;
    find_exact_state_timeout = 90;
    find_nearest_state_timeout = 30;
    current_hg_rev_timeout = 30;
    current_base_rev_timeout = 30;
  }
