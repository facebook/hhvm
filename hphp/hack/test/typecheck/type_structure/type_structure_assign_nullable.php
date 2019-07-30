<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function nullable<T>(TypeStructure<T> $ts): TypeStructure<?T> {
    $ts['nullable'] = true;
    /* HH_FIXME[4110] Replacing unsafe_expr */
    return $ts;
  }
