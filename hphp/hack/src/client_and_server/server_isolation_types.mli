(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A group of files that collectively form a leaf: they reference the monorepo,
    but nothing outside the group references them. *)
type cluster = {
  files: string list;
  common_directory: string;
      (** Longest directory prefix shared by every file in the cluster. *)
  truncated: bool;
      (** Whether a size cap refused this cluster a file it would otherwise have
          taken, leaving it a prefix of a package rather than a package. *)
}

type result = {
  refused: string option;
  clusters: cluster list;
  grown: bool;
      (** Whether these were grown, or are each the smallest isolatable set
          around a starting file. *)
  total_seeds: int;
  total_isolatable_files: int;
  total_clusters: int;
  total_truncated: int;
      (** Carried as a count rather than counted over [clusters], which is empty
          whenever the clusters were streamed to a file. *)
  largest_cluster: int;
}

(** Where to start growing clusters, and how far to take them. *)
type options = {
  no_growth: bool;
      (** Report each starting file's smallest isolatable set — its closure under
          reverse dependencies — instead of growing it. *)
  output_file: string option;
      (** Write each cluster as it is produced, one JSON object per line, and
          return only the summary. A family-sized run cannot hold every cluster
          in memory to send back through the RPC. *)
  seed_framework: string option;
      (** Start from every subclass of this class. *)
  seed_list: string option;
      (** Start from the repo-relative paths in this file, one per line. *)
  seed_dir: string option;
      (** Start from every Hack file under this repo-relative directory. *)
  max_dependents: int;
      (** The most files that may depend on a candidate for it to be absorbed
          along with them — how far the closure rule may reach. At least 1. *)
  max_cluster_size: int option;
      (** Stop growing a cluster at this many files and never report a larger
          one, marking it [truncated]. [None] grows to a fixed point. *)
  max_seeds: int option;  (** [None] means every seed. *)
  seed_offset: int;
      (** Skip this many seeds before applying [max_seeds], so a caller can walk
          the seed list in chunks across separate queries. *)
}
