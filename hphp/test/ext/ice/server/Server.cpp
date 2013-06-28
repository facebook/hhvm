// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <class_demoI.h>

using namespace std;

class ClassDemoServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    ClassDemoServer app;
    return app.main(argc, argv, "config.server");
}

int
ClassDemoServer::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("ClassDemo");
    com::jd::CommontIceRpcServicePtr classdemo = new com::jd::CommontIceRpcServiceI;
    adapter->add(classdemo, communicator()->stringToIdentity("classdemo"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}

