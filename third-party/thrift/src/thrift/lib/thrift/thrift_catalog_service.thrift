/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

include "thrift/lib/thrift/service_catalog.thrift"
include "thrift/lib/thrift/type_id.thrift"

package "facebook.com/thrift/catalog"

namespace cpp2 apache.thrift.catalog

exception CatalogUnavailable {
  1: string reason;
}

exception NotFound {
  1: string uri;
}

struct ThriftCatalogItem {
  1: binary digest;
  2: service_catalog.SerializableServiceCatalog catalog;
}

service ThriftCatalogService {
  // Returns the entire service catalog, including all services and types.
  //
  // Will throw `CatalogUnavailable` if no embedded catalog is available.
  ThriftCatalogItem getThriftServiceCatalog() throws (
    1: CatalogUnavailable unavailable,
  );

  // Returns the service catalog digest, i.e. the unique fingerprint
  // of the service catalog. This can be used to compare two service
  // catalogs to see if they are identical.
  //
  // Will throw `CatalogUnavailable` if no embedded catalog is available.
  binary getThriftServiceCatalogDigest() throws (
    1: CatalogUnavailable unavailable,
  );

  // Returns the service definition for the given service URI.
  // Note: This is wrapped in a ThriftCatalogItem to support
  // service hierarchies. Without a hierarchy, the catalog
  // will only contain a single ServiceDescriptor.
  //
  // Will throw `CatalogUnavailable` if no embedded service descriptor is available.
  // Will throw `NotFound` if the service is not found.
  ThriftCatalogItem getThriftServiceDescriptor(
    1: type_id.Uri serviceUri,
  ) throws (1: CatalogUnavailable unavailable, 2: NotFound notFound);

  // Returns the digest of the ServiceCatalog with the ThriftServiceDescriptor
  // as the root, i.e. the unique fingerprint of the service definition hierarchy. This can be used to compare two service
  // definitions to see if they are identical.
  //
  // Will throw `CatalogUnavailable` if no embedded service descriptor is available.
  // Will throw `NotFound` if the service is not found.
  binary getThriftServiceDescriptorDigest(1: type_id.Uri serviceUri) throws (
    1: CatalogUnavailable unavailable,
    2: NotFound notFound,
  );
}
