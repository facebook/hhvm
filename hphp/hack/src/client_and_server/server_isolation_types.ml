(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type cluster = {
  files: string list;
  common_directory: string;
  truncated: bool;
}

type result = {
  refused: string option;
  clusters: cluster list;
  grown: bool;
  total_seeds: int;
  total_isolatable_files: int;
  total_clusters: int;
  total_truncated: int;
  largest_cluster: int;
}

type options = {
  no_growth: bool;
  output_file: string option;
  seed_framework: string option;
  seed_list: string option;
  seed_dir: string option;
  max_dependents: int;
  max_cluster_size: int option;
  max_seeds: int option;
  seed_offset: int;
}
