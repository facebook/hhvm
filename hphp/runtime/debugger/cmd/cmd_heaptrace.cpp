/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "hphp/runtime/vm/unit.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdHeaptrace::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_accum.typesMap);
  thrift.write(m_accum.adjacencyList);
}

void CmdHeaptrace::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_accum.typesMap);
  thrift.read(m_accum.adjacencyList);
}

void CmdHeaptrace::list(DebuggerClient &client) {
}

void CmdHeaptrace::help(DebuggerClient &client) {
  client.helpTitle("Heaptrace Command");
  client.helpCmds(
    "[h]eaptrace",              "dumps all currently reachable values",
    "[h]eaptrace {filename}",   "dumps heap to GraphViz graph file",
    nullptr
  );
  client.helpBody(
    "This will print the locations and types of all reachable values "
    "in the heap."
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
    client.print(folly::stringPrintf("Found TV at %p with type %s (%u)",
                                     (void *)pair.first,
                                     typeName(pair.second),
                                     pair.second));
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

void CmdHeaptrace::printGraphToFile(DebuggerClient &client, String filename) {
  const char *name = filename->data();
  FILE *graphFile = fopen(name, "w");
  if (!graphFile) {
    client.print("Could not open file!");
    return;
  }

  fprintf(graphFile, "digraph {\n  node [shape=box];\n");
  for (const auto &pair : m_accum.typesMap) {
    void *ptr = (void *)pair.first;
    std::string n = folly::stringPrintf(
      "  node%p [label=\"TV at %p\\ntype %s\"];\n",
      ptr,
      ptr,
      typeName(pair.second)
    );
    fprintf(graphFile, "%s", n.c_str());
    std::vector<int64_t> &adjList = m_accum.adjacencyList[pair.first];
    for (const int64_t adjacent : adjList) {
      std::string e = folly::stringPrintf(
        "  node%p -> node%p;\n",
        ptr,
        (void *)adjacent
      );
      fprintf(graphFile, "%s", e.c_str());
    }
  }
  fprintf(graphFile, "}\n");
  fclose(graphFile);

  client.print(folly::stringPrintf("Wrote heap graph to %s.", name));
}

void CmdHeaptrace::onClientImpl(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;

  String file;
  if (client.argCount() == 2) {
    file = client.argValue(2);
  } else if (client.argCount() != 1) {
    help(client);
    return;
  }

  CmdHeaptracePtr cmd = client.xend<CmdHeaptrace>(this);

  if (file.empty()) {
    cmd->printHeap(client);
  } else {
    cmd->printGraphToFile(client, file);
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
