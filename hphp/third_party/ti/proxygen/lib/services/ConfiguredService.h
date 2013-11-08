// Copyright 2004-present Facebook. All Rights Reserved.
#pragma once

#include "ti/proxygen/lib/services/Service.h"

#include <memory>

namespace facebook { namespace proxygen {

class AccessLogHelper;
class ServiceConfiguration;

class ConfiguredService : public Service {
 public:
  explicit ConfiguredService(
    std::unique_ptr<ServiceConfiguration> config = nullptr);
  virtual ~ConfiguredService();

  /**
   * Service level configuration.
   *
   * The configuration object should derive from ServiceConfiguration
   * NOTE: This is not declared virtual so that respective Service's can
   *       down_cast it to appropriate Configuration object and pass on.
   */
  /* final */ const ServiceConfiguration *getConfig() const {
    return config_.get();
  }

  /**
   * Access log helper, if using one.
   */
  virtual AccessLogHelper* getAccessLogHelper() const {
    return nullptr;
  }

 private:
  // Forbidden copy constructor and assignment operator
  ConfiguredService(ConfiguredService const &) = delete;
  ConfiguredService& operator=(ConfiguredService const &) = delete;

  // Service configuration
  std::unique_ptr<ServiceConfiguration> config_;
};

}} // facebook::proxygen
