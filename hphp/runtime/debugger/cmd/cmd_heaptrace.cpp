/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/debugger/cmd/cmd_heaptrace.h"

#include "hphp/runtime/base/memory-profile.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

static GraphFormat GraphViz = {
  // prologue
  "digraph {\n  node [shape=box];\n",
  // node
  [](TypedValue *n, const char *type) {
    return folly::stringPrintf(
      "  node%p [label=\"TV at %p\\ntype %s\"];\n", n, n, type
    );
  },
  // edge
  [](TypedValue *u, TypedValue *v) {
    return folly::stringPrintf(
      "  node%p -> node%p;\n", u, v
    );
  },
  // epilogue
  "}\n"
};

static GraphFormat GML = {
  // prologue
  "graph [\n  directed 1\n",
  // node
  [](TypedValue *n, const char *type) {
    return folly::stringPrintf(
      "  node [\n"
      "    id \"node%p\"\n"
      "    label \"%s at %p\"\n"
      "  ]\n",
      n, type, n
    );
  },
  // edge
  [](TypedValue *u, TypedValue *v) {
    return folly::stringPrintf(
      "  edge [\n"
      "    source \"node%p\"\n"
      "    target \"node%p\"\n"
      "  ]\n",
      u,
      v
    );
  },
  // epilogue
  "]\n"
};

static std::map<std::string, GraphFormat> s_formatMap = {
  { "graphviz", GraphViz   },
  { "gml"     , GML        }
};

void CmdHeaptrace::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_accum.typesMap);
  thrift.write(m_accum.sizeMap);
  thrift.write(m_accum.adjacencyList);
}

void CmdHeaptrace::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_accum.typesMap);
  thrift.read(m_accum.sizeMap);
  thrift.read(m_accum.adjacencyList);
}

void CmdHeaptrace::list(DebuggerClient &client) {
}

void CmdHeaptrace::help(DebuggerClient &client) {
  client.helpTitle("Heaptrace Command");
  client.helpCmds(
    "[h]eaptrace",                     "dumps all currently reachable values",
    "[h]eaptrace {format} {filename}", "dumps heap to graph file",
    nullptr
  );
  client.helpBody(
    "This will print the locations and types of all reachable values "
    "in the heap. The long form dumps it to a file of a supported format.\n"
    "Supported formats are currently:\n"
    " - graphviz : Dumps to a GraphViz file, which can be used with e.g. "
    "dot, twopi or some other GraphViz tool to render an image.\n"
    " - gml      : Dumps to a GML (Graph Modelling Language) file, which can "
    "be viewed interactively with programs like yEd. yEd can be found at "
    "www.yworks.com."
  );
}

static const char *typeName(int8_t type) {
  switch (type) {
    case KindOfUninit:
      return "uninit";
    case KindOfNull:
      return "null";
    case KindOfBoolean:
      return "boolean";
    case KindOfInt64:
      return "integer";
    case KindOfDouble:
      return "double";
    case KindOfStaticString:
      return "string (static)";
    case KindOfString:
      return "string";
    case KindOfArray:
      return "array";
    case KindOfObject:
      return "object";
    case KindOfResource:
      return "resource";
    case KindOfRef:
      return "reference";
    case KindOfIndirect:
      return "indirect";
    default:
      return "unknown";
  }
}

void CmdHeaptrace::printHeap(DebuggerClient &client) {
  for (const auto &pair : m_accum.typesMap) {
    size_t size = m_accum.sizeMap[pair.first];
    std::string sizeStr = size
      ? folly::stringPrintf(" which consumes %lu bytes", size)
      : std::string();
    client.print(
      folly::stringPrintf("Found TV at %p with type %s%s",
                          (void *)pair.first,
                          typeName(pair.second),
                          sizeStr.c_str()));
    std::vector<int64_t> &adjList = m_accum.adjacencyList[pair.first];
    if (!adjList.empty()) {
      std::string children = "  -> found children: ";
      bool first = true;
      for (const int64_t &adjacent : adjList) {
        if (!first) {
          children += ", ";
        }
        children += folly::stringPrintf("%p", (void *)adjacent);
        first = false;
      }
      client.print(children);
    }
  }
}

void CmdHeaptrace::printGraphToFile(DebuggerClient &client,
                                    String filename,
                                    const GraphFormat &gf) {
  const char *name = filename->data();
  FILE *graphFile = fopen(name, "w");
  if (!graphFile) {
    client.print("Could not open file!");
    return;
  }

  fprintf(graphFile, "%s", gf.prologue.c_str());
  for (const auto &pair : m_accum.typesMap) {
    std::string n = gf.stringifyNode((TypedValue *)pair.first,
                                     typeName(pair.second));
    fprintf(graphFile, "%s", n.c_str());
    std::vector<int64_t> &adjList = m_accum.adjacencyList[pair.first];
    for (const int64_t adjacent : adjList) {
      std::string e = gf.stringifyEdge((TypedValue *)pair.first,
                                       (TypedValue *)adjacent);
      fprintf(graphFile, "%s", e.c_str());
    }
  }
  fprintf(graphFile, "%s", gf.epilogue.c_str());
  fclose(graphFile);

  client.print(folly::stringPrintf("Wrote heap graph to %s.", name));
}

void CmdHeaptrace::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;

  String format;
  String file;
  if (client.argCount() == 3) {
    format = client.argValue(2);
    file = client.argValue(3);
  } else if (client.argCount() != 1) {
    help(client);
    return;
  }

  CmdHeaptracePtr cmd = client.xend<CmdHeaptrace>(this);

  if (file.empty()) {
    cmd->printHeap(client);
  } else {
    std::string formatStr = format->data();
    const auto it = s_formatMap.find(formatStr);

    if (it == s_formatMap.end()) {
      client.print("Unsupported format type");
      return;
    }
    cmd->printGraphToFile(client, file, it->second);
  }

}

bool CmdHeaptrace::onServer(DebuggerProxy &proxy) {

  // globals
  std::vector<TypedValue *> roots;
  CArrRef arr = g_vmContext->m_globalVarEnv->getDefinedVariables();
  arr->getChildren(roots);

  // static properties
  for (AllClasses ac; !ac.empty();) {
    Class *c = ac.popFront();
    c->getChildren(roots);
  }

  // locals
  int numFrames = proxy.getRealStackDepth();
  std::vector<Array> locs;
  for (int i = 0; i < numFrames; ++i) {
    locs.push_back(g_vmContext->getLocalDefinedVariables(i));
  }
  for (CArrRef locArr : locs) {
    locArr->getChildren(roots);
  }

  Tracer<Accum>::traceAll(
    roots,
    [](TypedValue *node, Accum &accum) {
      accum.typesMap[(int64_t)node] = (int8_t)node->m_type;
      accum.sizeMap[(int64_t)node] = (int64_t)MemoryProfile::getSizeOfTV(node);
    },
    [](TypedValue *parent, TypedValue *child, Accum &accum) {
      accum.adjacencyList[(int64_t)parent].push_back((int64_t)child);
    },
    m_accum
  );

  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
