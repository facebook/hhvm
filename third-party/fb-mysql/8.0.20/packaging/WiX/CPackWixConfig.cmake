# Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

SET(CPACK_COMPONENTS_USED 
    "Server;Client;DataFiles;Development;SharedLibraries;Documentation;IniFiles;Readme;Server_Scripts;Meb;MebReadme;Router")

IF(WITH_NDBCLUSTER)
  MESSAGE(STATUS "This is Cluster build, append additional components")
  SET(CPACK_COMPONENTS_USED
    "${CPACK_COMPONENTS_USED};ClusterTools;ClusterDataNode;ClusterManagementServer;ClusterManagementClient;ClusterJ;nodejs")
ENDIF()

# Some components are optional
# We will build MSI without optional components that are not selected for build
#(need to modify CPACK_COMPONENTS_ALL for that)
SET(CPACK_ALL)
FOREACH(comp1 ${CPACK_COMPONENTS_USED})
 SET(found)
 FOREACH(comp2 ${CPACK_COMPONENTS_ALL})
  IF(comp1 STREQUAL comp2)
    SET(found 1)
    BREAK()
  ENDIF()
 ENDFOREACH()
 IF(found)
   SET(CPACK_ALL ${CPACK_ALL} ${comp1})
 ENDIF()
ENDFOREACH()
SET(CPACK_COMPONENTS_ALL ${CPACK_ALL})

# Always install (hidden), includes Readme files
SET(CPACK_COMPONENT_GROUP_ALWAYSINSTALL_HIDDEN 1)
SET(CPACK_COMPONENT_README_GROUP "AlwaysInstall")

# Feature MySQL Server
SET(CPACK_COMPONENT_GROUP_MYSQLSERVER_DISPLAY_NAME "MySQL Server")
SET(CPACK_COMPONENT_GROUP_MYSQLSERVER_EXPANDED "1")
SET(CPACK_COMPONENT_GROUP_MYSQLSERVER_DESCRIPTION "Install MySQL Server")
 # Subfeature "Server" (hidden)
 SET(CPACK_COMPONENT_SERVER_GROUP "MySQLServer")
 SET(CPACK_COMPONENT_SERVER_HIDDEN 1)
 # Subfeature "Shared libraries" (hidden)
 SET(CPACK_COMPONENT_SHAREDLIBRARIES_GROUP "MySQLServer")
 SET(CPACK_COMPONENT_SHAREDLIBRARIES_HIDDEN 1)
 # Subfeature "Client" 
 SET(CPACK_COMPONENT_CLIENT_GROUP "MySQLServer")
 SET(CPACK_COMPONENT_CLIENT_DISPLAY_NAME "Client Programs")
 SET(CPACK_COMPONENT_CLIENT_DESCRIPTION 
   "Various helpful (commandline) tools including the mysql command line client" )

 # Subfeature "Meb" 
 SET(CPACK_COMPONENT_GROUP_ALWAYSINSTALL_HIDDEN 1)
 SET(CPACK_COMPONENT_MEB_GROUP "AlwaysInstall")
 # Subfeature "MebReadme" 
 SET(CPACK_COMPONENT_GROUP_ALWAYSINSTALL_HIDDEN 1)
 SET(CPACK_COMPONENT_MEBREADME_GROUP "AlwaysInstall")

 #Subfeature MySQL Router
 SET(CPACK_COMPONENT_GROUP_ALWAYSINSTALL_HIDDEN 1)
 SET(CPACK_COMPONENT_ROUTER_GROUP "AlwaysInstall")
    
 #Subfeature "Data Files" 
 SET(CPACK_COMPONENT_DATAFILES_GROUP "MySQLServer")
 SET(CPACK_COMPONENT_DATAFILES_DISPLAY_NAME "Server data files")
 SET(CPACK_COMPONENT_DATAFILES_DESCRIPTION "Server data files" )
 SET(CPACK_COMPONENT_DATAFILES_HIDDEN 1)
  
#Feature "Devel"
SET(CPACK_COMPONENT_GROUP_DEVEL_DISPLAY_NAME "Development Components")
SET(CPACK_COMPONENT_GROUP_DEVEL_DESCRIPTION "Installs C/C++ header files and libraries")
 #Subfeature "Development"
 SET(CPACK_COMPONENT_DEVELOPMENT_GROUP "Devel")
 SET(CPACK_COMPONENT_DEVELOPMENT_HIDDEN 1)
  
 #Subfeature "Embedded"
 SET(CPACK_COMPONENT_EMBEDDED_GROUP "Devel")
 SET(CPACK_COMPONENT_EMBEDDED_DISPLAY_NAME "Embedded server library")
 SET(CPACK_COMPONENT_EMBEDDED_DESCRIPTION "Installs embedded server library")
 SET(CPACK_COMPONENT_EMBEDDED_WIX_LEVEL 2)

#Feature Debug Symbols
SET(CPACK_COMPONENT_GROUP_DEBUGSYMBOLS_DISPLAY_NAME "Debug Symbols")
SET(CPACK_COMPONENT_GROUP_DEBUGSYMBOLS_DESCRIPTION "Installs Debug Symbols")
SET(CPACK_COMPONENT_GROUP_DEBUGSYMBOLS_WIX_LEVEL 2)
 SET(CPACK_COMPONENT_DEBUGINFO_GROUP "DebugSymbols")
 SET(CPACK_COMPONENT_DEBUGINFO_HIDDEN 1)

#Feature Documentation
SET(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
SET(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "Installs documentation")
SET(CPACK_COMPONENT_DOCUMENTATION_WIX_LEVEL 2)

#Feature tests
SET(CPACK_COMPONENT_TEST_DISPLAY_NAME "Tests")
SET(CPACK_COMPONENT_TEST_DESCRIPTION "Installs unittests (requires Perl to run)")
SET(CPACK_COMPONENT_TEST_WIX_LEVEL 2)


#Feature Misc (hidden, installs only if everything is installed)
SET(CPACK_COMPONENT_GROUP_MISC_HIDDEN 1)
SET(CPACK_COMPONENT_GROUP_MISC_WIX_LEVEL 100)
  SET(CPACK_COMPONENT_INIFILES_GROUP "Misc")
  SET(CPACK_COMPONENT_SERVER_SCRIPTS_GROUP "Misc")

IF(WITH_NDBCLUSTER)
  MESSAGE(STATUS "This is Cluster build, define additional components")
  #Feature "Cluster"
  SET(CPACK_COMPONENT_GROUP_CLUSTER_DISPLAY_NAME "MySQL Cluster")
  SET(CPACK_COMPONENT_GROUP_CLUSTER_DESCRIPTION "Installs MySQL Cluster")

  #Subfeature "ClusterTools"
  SET(CPACK_COMPONENT_CLUSTERTOOLS_GROUP "Cluster")
  SET(CPACK_COMPONENT_CLUSTERTOOLS_DISPLAY_NAME "Cluster Tools")
  SET(CPACK_COMPONENT_CLUSTERTOOLS_DESCRIPTION "Installs Cluster Tools")

  #Subfeature "Cluster Storage Engines"
  SET(CPACK_COMPONENT_CLUSTERDATANODE_GROUP "Cluster")
  SET(CPACK_COMPONENT_CLUSTERDATANODE_DISPLAY_NAME "Cluster Storage Engines")
  SET(CPACK_COMPONENT_CLUSTERDATANODE_DESCRIPTION "Installs Cluster Storage Engines")

  #Subfeature "Cluster Management Server"
  SET(CPACK_COMPONENT_CLUSTERMANAGEMENTSERVER_GROUP "Cluster")
  SET(CPACK_COMPONENT_CLUSTERMANAGEMENTSERVER_DISPLAY_NAME "Cluster Management Server")
  SET(CPACK_COMPONENT_CLUSTERMANAGEMENTSERVER_DESCRIPTION "Installs Cluster Management Server")

  #Subfeature "Cluster Management Client"^M
  SET(CPACK_COMPONENT_CLUSTERMANAGEMENTCLIENT_GROUP "Cluster")
  SET(CPACK_COMPONENT_CLUSTERMANAGEMENTCLIENT_DISPLAY_NAME "Cluster Management Client")
  SET(CPACK_COMPONENT_CLUSTERMANAGEMENTCLIENT_DESCRIPTION "Installs Cluster Management Client")

  #Subfeature "ClusterJ"
  SET(CPACK_COMPONENT_CLUSTERJ_GROUP "Devel")
  SET(CPACK_COMPONENT_CLUSTERJ_DISPLAY_NAME "ClusterJ Java Connector for Cluster")
  SET(CPACK_COMPONENT_CLUSTERJ_DESCRIPTION "Installs ClusterJ")

  #Subfeature "nodejs"
  SET(CPACK_COMPONENT_NODEJS_GROUP "Devel")
  SET(CPACK_COMPONENT_NODEJS_DISPLAY_NAME "nodejs Connector for Cluster")
  SET(CPACK_COMPONENT_NODEJS_DESCRIPTION "Installs nodejs connector")
ENDIF()
