/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTPSettings.h>

#include <algorithm>

namespace proxygen {

void HTTPSettings::setSetting(SettingsId id, SettingsValue val) {
  auto iter = getSettingIter(id);
  if (iter != settings_.end()) {
    (*iter).value = val;
  } else {
    settings_.emplace_back(id, val);
  }
}

void HTTPSettings::unsetSetting(SettingsId id) {
  auto iter = getSettingIter(id);
  if (iter != settings_.end()) {
    *iter = settings_.back();
    settings_.pop_back();
  }
}

const HTTPSetting* HTTPSettings::getSetting(SettingsId id) const {
  auto iter = getSettingConstIter(id);
  if (iter != settings_.end()) {
    return &(*iter);
  } else {
    return nullptr;
  }
}

SettingsValue HTTPSettings::getSetting(SettingsId id,
                                       SettingsValue defaultValue) const {
  auto iter = getSettingConstIter(id);
  if (iter != settings_.end()) {
    return (*iter).value;
  } else {
    return defaultValue;
  }
}

std::vector<HTTPSetting>::iterator HTTPSettings::getSettingIter(SettingsId id) {
  return std::find_if(settings_.begin(),
                      settings_.end(),
                      [&](HTTPSetting const& s) { return s.id == id; });
}

std::vector<HTTPSetting>::const_iterator HTTPSettings::getSettingConstIter(
    SettingsId id) const {
  return std::find_if(settings_.begin(),
                      settings_.end(),
                      [&](HTTPSetting const& s) { return s.id == id; });
}

} // namespace proxygen
