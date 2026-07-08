include "thrift/annotation/hack.thrift"

@hack.NamePrefix{prefix = "EchoModule_", apply_to_services = true}
@hack.LegacyAlwaysIncludeNamePrefixInProcessor
package;

service EchoService {
  string echo(1: string msg);
}
