# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

SUBDIRS = lib . routes tools test

ACLOCAL_AMFLAGS = -I m4

noinst_LIBRARIES = libmcroutercore.a
bin_PROGRAMS = mcrouter

BUILT_SOURCES = \
  lib/network/McAsciiParser-gen.cpp

lib/network/McAsciiParser-gen.cpp: lib/network/McAsciiParser.rl
	ragel -G1 -o lib/network/McAsciiParser-gen.cpp lib/network/McAsciiParser.rl

CLEANFILES = \
  lib/network/McAsciiParser-gen.cpp

libmcroutercore_a_SOURCES = \
  AsyncLog.cpp \
  AsyncLog.h \
  AsyncWriter.cpp \
  AsyncWriter.h \
  AsyncWriterEntry.h \
  CallbackPool-inl.h \
  CallbackPool.h \
  CarbonRouterClient.cpp \
  CarbonRouterClient-inl.h \
  CarbonRouterClient.h \
  CarbonRouterClientBase.cpp \
  CarbonRouterClientBase.h \
  CarbonRouterFactory.h \
  CarbonRouterFactory.cpp \
  CarbonRouterInstance-inl.h \
  CarbonRouterInstance.cpp \
  CarbonRouterInstance.h \
  CarbonRouterInstanceBase.cpp \
  CarbonRouterInstanceBase.h \
  ConfigApi.cpp \
  ConfigApi.h \
  ConfigApiIf.h \
  ExecutorObserver.h \
  ExponentialSmoothData.h \
  FileDataProvider.cpp \
  FileDataProvider.h \
  FileObserver.cpp \
  FileObserver.h \
  ForEachPossibleClient.h \
  flavor.cpp \
  flavor.h \
  LeaseTokenMap.cpp \
  LeaseTokenMap.h \
  mcrouter_config-impl.h \
  mcrouter_config.cpp \
  mcrouter_config.h \
  mcrouter_options_list.h \
  McrouterClient.h \
  McrouterFiberContext.cpp \
  McrouterFiberContext.h \
  McrouterInstance.h \
  McrouterLogFailure.cpp \
  McrouterLogFailure.h \
  McrouterLogger.cpp \
  McrouterLogger.h \
  McrouterManager.cpp\
  McrouterManager.h\
  McSpoolUtils.h \
  McSpoolUtils.cpp \
  lib/invalidation/McInvalidationKvPairs.h \
  lib/invalidation/McInvalidationKvPairs.cpp \
  lib/invalidation/McInvalidationDefs.h \
  lib/network/McThriftContext.h \
  Observable-inl.h \
  Observable.h \
  options-template.h \
  options.cpp \
  options.h \
  OptionsUtil.cpp \
  OptionsUtil.h \
  PoolFactory.cpp \
  PoolFactory.h \
  Proxy-inl.h \
  Proxy.h \
  ProxyBase-inl.h \
  ProxyBase.cpp \
  ProxyBase.h \
  ProxyConfig-inl.h \
  ProxyConfig.h \
  ProxyConfigBuilder.cpp \
  ProxyConfigBuilder.h \
  ProxyDestination.cpp \
  ProxyDestination-inl.h \
  ProxyDestination.h \
  ProxyDestinationBase.cpp \
  ProxyDestinationBase.h \
  ProxyDestinationKey.cpp \
  ProxyDestinationKey.h \
  ProxyDestinationMap-inl.h \
  ProxyDestinationMap.cpp \
  ProxyDestinationMap.h \
  ProxyRequestContext.cpp \
  ProxyRequestContext.h \
  ProxyRequestContextTyped-inl.h \
  ProxyRequestContextTyped.h \
  ProxyRequestLogger-inl.h \
  ProxyRequestLogger.h \
  ProxyRequestPriority.h \
  ProxyStats.cpp \
  ProxyStats.h \
  route.cpp \
  route.h \
  routes/AllAsyncRouteFactory.h \
  routes/AllFastestRouteFactory.h \
  routes/AllInitialRouteFactory.h \
  routes/AllMajorityRouteFactory.h \
  routes/AllSyncRouteFactory.h \
  routes/AsynclogRoute.h \
  routes/BigValueRoute-inl.h \
  routes/BigValueRoute.cpp \
  routes/BigValueRoute.h \
  routes/BigValueRouteIf.h \
  routes/CarbonLookasideRoute.h \
  routes/CarbonLookasideRoute.cpp \
  routes/DefaultShadowPolicy.h \
  routes/DestinationRoute.h \
  routes/DevNullRoute.h \
  routes/ErrorRoute.h \
  routes/ExtraRouteHandleProviderIf.h \
  routes/FailoverPolicy.h \
  routes/FailoverRateLimiter.cpp \
  routes/FailoverRateLimiter.h \
  routes/FailoverRoute-inl.h \
  routes/FailoverRoute.h \
  routes/FailoverWithExptimeRouteFactory.h \
  routes/HostIdRouteFactory.h \
  routes/KeySplitRoute-inl.h \
  routes/KeySplitRoute.h \
  routes/L1L2CacheRouteFactory.h \
  routes/L1L2SizeSplitRoute-inl.h \
  routes/L1L2SizeSplitRoute.h \
  routes/LatencyInjectionRoute.h \
  routes/LatencyInjectionRoute.cpp \
  routes/LatestRoute.h \
  routes/McBucketRoute-inl.h \
  routes/McBucketRoute.cpp \
  routes/McBucketRoute.h \
  routes/DistributionRoute-inl.h \
  routes/DistributionRoute.h \
  routes/McExtraRouteHandleProvider-inl.h \
  routes/McExtraRouteHandleProvider.h \
  routes/McImportResolver.cpp \
  routes/McImportResolver.h \
  routes/McRouteHandleBuilder.h \
  routes/McRouteHandleProvider-inl.h \
  routes/McRouteHandleProvider.cpp \
  routes/McRouteHandleProvider-AllFastestRoute.cpp \
  routes/McRouteHandleProvider-CarbonLookasideRoute.cpp \
  routes/McRouteHandleProvider-FailoverRoute.cpp \
  routes/McRouteHandleProvider-HashRoute.cpp \
  routes/McRouteHandleProvider-PoolRoute.cpp \
  routes/McRouteHandleProvider.h \
  routes/McrouterRouteHandle.h \
  routes/MigrateRouteFactory.h \
  routes/MissFailoverRoute.h \
  routes/ModifyExptimeRoute.h \
  routes/ModifyKeyRoute.h \
  routes/NullRoute.cpp \
  routes/OperationSelectorRoute-inl.h \
  routes/OperationSelectorRoute.h \
  routes/OriginalClientHashRoute.h \
  routes/OutstandingLimitRoute.h \
  routes/PoolRouteUtils.h \
  routes/PrefixSelectorRoute.h \
  routes/ProxyRoute-inl.h \
  routes/ProxyRoute.h \
  routes/RandomRouteFactory.h \
  routes/RateLimiter.cpp \
  routes/RateLimiter.h \
  routes/RendezvousRouteHelpers.cpp \
  routes/RendezvousRouteHelpers.h \
  routes/RateLimitRoute.h \
  routes/RootRoute.h \
  routes/RouteHandleMap-inl.h \
  routes/RouteHandleMap.h \
  routes/RoutePolicyMap-inl.h \
  routes/RoutePolicyMap.h \
  routes/RouteSelectorMap.h \
  routes/ShadowRoute.h \
  routes/ShadowRoute-inl.h \
  routes/ShadowRouteIf.h \
  routes/ShadowSettings.cpp \
  routes/ShadowSettings.h \
  routes/ShardHashFunc.cpp \
  routes/ShardHashFunc.h \
  routes/ShardSelectionRouteFactory.h \
  routes/ShardSelectionRouteFactory-inl.h \
  routes/ShardSelectionRouteFactory.cpp \
  routes/ShardSplitRoute.cpp \
  routes/ShardSplitRoute.h \
  routes/ShardSplitter.cpp \
  routes/ShardSplitter.h \
  routes/SlowWarmupRoute.h \
  routes/SlowWarmUpRouteSettings.cpp \
  routes/SlowWarmupRouteSettings.h \
  routes/StagingRoute.cpp \
  routes/StagingRoute.h \
  routes/TimeProviderFunc.h \
  routes/WarmUpRoute.cpp \
  routes/WarmUpRoute.h \
  RouterRegistry-impl.h \
  RoutingPrefix.cpp \
  RoutingPrefix.h \
  RuntimeVarsData.cpp \
  RuntimeVarsData.h \
  ServiceInfo.cpp \
  ServiceInfo-inl.h \
  ServiceInfo.h \
  stat_list.h \
  stats.cpp \
  stats.h \
  ThreadUtil.cpp \
  ThreadUtil.h \
  ThriftAcceptor.h \
  ThriftAcceptor.cpp \
  TkoCounters.h \
  TkoLog.cpp \
  TkoLog.h \
  TkoTracker.cpp \
  TkoTracker.h \
  ExternalStatsHandler.cpp \
  ExternalStatsHandler.h

mcrouter_SOURCES = \
  main.cpp \
  Server-inl.h \
  Server.h \
  ServerOnRequest.h \
  RequestAclChecker.h \
  RequestAclChecker.cpp \
  StandaloneConfig.cpp \
  StandaloneConfig.h \
  StandaloneUtils.cpp \
  StandaloneUtils.h \
  standalone_options.h \
  standalone_options_list.h

libmcroutercore_a_CPPFLAGS = -I..

# Link order matters.
mcrouter_LDADD = \
  libmcroutercore.a \
  lib/libmcrouter.a \
  -lthriftcpp2 \
  -ltransport \
  -lthriftanyrep \
  -lthrifttype \
  -lthrifttyperep \
  -lthriftprotocol \
  -lrpcmetadata \
  -lthriftannotation \
  -lthriftmetadata \
  -lasync \
  -lconcurrency \
  -lthrift-core \
	-lfmt \
  -lwangle \
  -lfolly

mcrouter_CPPFLAGS = -I..

clean-local:
	-rm -rf lib/network/gen-cpp2/ \
	        lib/carbon/gen-cpp2/

distclean-local:
	-rm -rf lib/network/gen-cpp2/ \
		lib/carbon/gen-cpp2/
