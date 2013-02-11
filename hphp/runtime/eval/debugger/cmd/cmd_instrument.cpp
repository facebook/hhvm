/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/eval/debugger/cmd/cmd_instrument.h>
#include <runtime/vm/instrumentation.h>
#include <runtime/ext/ext_file.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdInstrument::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_type);
  thrift.write(m_enabled);
  assert(m_instPoints);
  InstPointInfo::SendImpl(*m_instPoints, thrift);
}

void CmdInstrument::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_type);
  thrift.read(m_enabled);
  InstPointInfo::RecvImpl(m_ips, thrift);
}

bool CmdInstrument::help(DebuggerClient *client) {
  client->helpTitle("Instrument Command");
  // TODO: more functionalities
  client->helpCmds("inst here <file> [desc]",
                   "inject <file> to here",
                   "inst <func>() <file> [desc]",
                   "inject <file> to the entry point of <func>",
                   "inst [l]ist",
                   "list injections",
                   "inst [c]lear",
                   "clear all injections",
                   NULL);
  client->helpBody(
    "Use this command to instrument the program"
  );
  return true;
}

bool CmdInstrument::onClientVM(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() == 1) {
    if (client->argValue(1) == "list" || client->argValue(1) == "l") {
      listInst(client);
      return true;
    }
    if (client->argValue(1) == "clear" || client->argValue(1) == "c") {
      clearInst(client);
      return true;
    }
  }
  if (client->argCount() < 2 || client->argValue(1) == "help") {
    return help(client);
  }

  std::string loc = client->argValue(1);
  std::string file = client->argValue(2);
  std::string desc;
  if (client->argCount() >= 3) {
    desc = client->argValue(3);
  }
  Variant code = f_file_get_contents(file.c_str());
  if (code.isNull()) {
    client->error("Unable to read from file %s", file.c_str());
    return false;
  }
  m_instPoints = client->getInstPoints();
  if (loc == "here") {
    InstPointInfoPtr ipi(new InstPointInfo());
    ipi->setLocHere();
    ipi->m_code = code.toString();
    ipi->m_desc = desc;
    m_instPoints->push_back(ipi);
  } else if (loc.rfind("()") == loc.size() - 2){
    InstPointInfoPtr ipi(new InstPointInfo());
    ipi->setLocFuncEntry(loc.substr(0, loc.size() - 2));
    ipi->m_code = code.toString();
    ipi->m_desc = desc;
    m_instPoints->push_back(ipi);
  } else {
    client->error("Not implemented\n");
    return true;
  }
  m_type = ActionWrite;
  CmdInstrumentPtr instCmdPtr = client->xend<CmdInstrument>(this);
  if (!instCmdPtr->m_enabled) {
    client->error("Instrumentation is not enabled on the server");
  }
  client->setInstPoints(instCmdPtr->m_ips);
  CmdInstrument::PrintInstPoints(client);
  return true;
}

void CmdInstrument::setClientOutput(DebuggerClient *client) {
  // Output all instrumentation point info
  client->setOutputType(DebuggerClient::OTValues);
  Array values;
  InstPointInfoPtrVec* ips = client->getInstPoints();
  for (unsigned int i = 0; i < ips->size(); i++) {
    InstPointInfoPtr ipi = (*ips)[i];
    Array instpoint;
    instpoint.set("valid", ipi->m_valid);
    instpoint.set("desc", ipi->m_desc);
    if (ipi->m_locType == InstPointInfo::LocFileLine) {
      instpoint.set("type", "file_line");
      instpoint.set("file", ipi->m_file);
      instpoint.set("line", ipi->m_line);
    } else if (ipi->m_locType == InstPointInfo::LocFuncEntry) {
      instpoint.set("type", "func_entry");
      instpoint.set("func", ipi->m_func);
    }
    values.append(instpoint);
  }
  client->setOTValues(values);
}

bool CmdInstrument::onServerVM(DebuggerProxy *proxy) {
  m_instPoints = &m_ips;
  m_enabled = true;
  DebuggerProxyVM* proxyVM = static_cast<DebuggerProxyVM*>(proxy);
  if (m_type == ActionRead) {
    readFromTable(proxyVM);
  } else if (m_type == ActionWrite) {
    validateAndWriteToTable(proxyVM);
  }
  return proxy->send(this);
}

void CmdInstrument::readFromTable(DebuggerProxyVM *proxy) {
  proxy->readInjTablesFromThread();
  m_ips.clear();
  if (!proxy->getInjTables()) {
    // nothing there
    return;
  }
  // Bytecode address
  VM::InjectionTableInt64* tablePC =
    proxy->getInjTables()->getInt64Table(VM::InstHookTypeBCPC);
  if (tablePC) {
    for (VM::InjectionTableInt64::const_iterator it = tablePC->begin();
         it != tablePC->end(); ++it) {
      const VM::Injection* inj = it->second;
      InstPointInfoPtr ipi(new InstPointInfo());
      ipi->m_valid = true;
      if (inj->m_desc) {
        ipi->m_desc = inj->m_desc->data();
      }
      ipi->m_locType = InstPointInfo::LocFileLine;
      // TODO use pc to figure out m_file and m_line
      // uchar* pc = (uchar*)it->first;
      m_ips.push_back(ipi);
    }
  }
  VM::InjectionTableSD* tableFEntry =
    proxy->getInjTables()->getSDTable(VM::InstHookTypeFuncEntry);
  if (tableFEntry) {
    for (VM::InjectionTableSD::const_iterator it = tableFEntry->begin();
         it != tableFEntry->end(); ++it) {
      const VM::Injection* inj = it->second;
      InstPointInfoPtr ipi(new InstPointInfo());
      ipi->m_valid = true;
      if (inj->m_desc) {
        ipi->m_desc = inj->m_desc->data();
      }
      ipi->m_func = it->first->data();
      ipi->m_locType = InstPointInfo::LocFuncEntry;
      m_ips.push_back(ipi);
    }
  }
}

void CmdInstrument::validateAndWriteToTable(DebuggerProxyVM *proxy) {
  if (!proxy->getInjTables()) {
    proxy->setInjTables(new VM::InjectionTables());
  }
  VM::InjectionTableInt64* tablePC = NULL;
  VM::InjectionTableSD* tableFEntry = NULL;

  for (int i = 0; i < (int)m_ips.size(); i++) {
    InstPointInfoPtr ipi = m_ips[i];
    const VM::Injection* inj =
      VM::InjectionCache::GetInjection(ipi->m_code, ipi->m_desc);
    if (!inj) { // error in the code
      continue;
    }
    if (ipi->m_locType == InstPointInfo::LocHere ||
        ipi->m_locType == InstPointInfo::LocFileLine) {
      // bytecode address
      const uchar *pc = ipi->lookupPC();
      if (pc == NULL) {
        continue;
      }
      if (tablePC == NULL) {
        tablePC = new VM::InjectionTableInt64();
      }
      ipi->m_valid = true;
      (*tablePC)[(int64)pc] = inj;
    }
    if (ipi->m_locType == InstPointInfo::LocFuncEntry) {
      StackStringData sd(ipi->m_func.c_str(), ipi->m_func.size(), AttachLiteral);
      const StringData* sdCache = VM::InjectionCache::GetStringData(&sd);
      if (tableFEntry == NULL) {
        tableFEntry = new VM::InjectionTableSD();
      }
      ipi->m_valid = true;
      (*tableFEntry)[sdCache] = inj;
    }
  }

  proxy->getInjTables()->setInt64Table(VM::InstHookTypeBCPC, tablePC);
  proxy->getInjTables()->setSDTable(VM::InstHookTypeFuncEntry, tableFEntry);

  proxy->writeInjTablesToThread();
}

void CmdInstrument::listInst(DebuggerClient *client) {
  m_type = ActionRead;
  m_instPoints = client->getInstPoints();
  CmdInstrumentPtr instCmdPtr = client->xend<CmdInstrument>(this);
  client->setInstPoints(instCmdPtr->m_ips);
  PrintInstPoints(client);
}

void CmdInstrument::clearInst(DebuggerClient *client) {
  m_type = ActionWrite;
  m_instPoints = client->getInstPoints();
  m_instPoints->clear();
  CmdInstrumentPtr instCmdPtr = client->xend<CmdInstrument>(this);
  client->setInstPoints(instCmdPtr->m_ips);
  PrintInstPoints(client);
}

void CmdInstrument::PrintInstPoints(DebuggerClient *client) {
  InstPointInfoPtrVec* ips = client->getInstPoints();
  int size = ips->size();
  client->print("%d instrumentation points", size);
  for (int i = 0; i < size; i++) {
    InstPointInfoPtr ipi = (*ips)[i];
    if (ipi->m_locType == InstPointInfo::LocFileLine) {
      client->print("  %d\t%s\t%s\tfile:\t%s:%d", i,
                    ipi->m_valid ? "valid" : "invalid",
                    ipi->m_desc.c_str(), ipi->m_file.c_str(), ipi->m_line);
    } else if (ipi->m_locType == InstPointInfo::LocFuncEntry) {
      client->print("  %d\t%s\t%s\tfunc entry:\t%s", i,
                    ipi->m_valid ? "valid" : "invalid",
                    ipi->m_desc.c_str(), ipi->m_func.c_str());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
