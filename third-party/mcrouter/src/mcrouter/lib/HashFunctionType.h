/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

/**
 * Types of Hash Functions
 */
enum class HashFunctionType : uint8_t {
  Unknown = 0,
  CH3 = 1,
  ConstShard = 2,
  CRC32 = 3,
  Rendezvous = 4,
  WeightedCh3 = 5,
  WeightedCh4 = 6,
  WeightedRendezvous = 7,
};
