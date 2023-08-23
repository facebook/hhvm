Facebook Thrift Go
--------------------------------------------

Thrift is a serialization and RPC framework for service communication. Thrift enables these features in all major languages, and there is strong support for C++, Python, Hack, and Java. Most services at Facebook are written using Thrift for RPC, and some storage systems use Thrift for serializing records on disk.

Facebook Thrift is not a distribution of [Apache Thrift](https://thrift.apache.org/). This is an evolved internal branch of Thrift that Facebook re-released to open source community in February 2014. Facebook Thrift was originally released closely tracking Apache Thrift but is now evolving in new directions. In particular, the compiler was rewritten from scratch and the new implementation features a fully asynchronous Thrift server. Read more about these improvements in the [ThriftServer documentation](https://github.com/facebook/fbthrift/blob/main/thrift/doc/intro.md).

You can also learn more about this project in the original Facebook Code [blog post](https://code.facebook.com/posts/1468950976659943/under-the-hood-building-and-open-sourcing-fbthrift/)

The files in this repository is only for the Go libraries of fbthrift, see [facebook/fbthrift](https://github.com/facebook/fbthrift) for the remaining code.

---

<img src="https://avatars2.githubusercontent.com/u/69631?s=200&v=4" width="50"></img>
