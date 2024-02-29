Simple Binary Encoder
=====================

Overview
--------

To work with SBE you will have to go through the following steps:
1) define a SBE schema describing your messages
2) use SBE Tool to generate the encoders and decoders (C# classes) for your messages
3) add the generated files to your project and a reference to SBE.dll 

To get you up and running quickly we will go through a quick sample which should give you a better idea of how to use SBE.
If you have used Google Protocol Buffer in the past, this should sound familiar... if not, don't worry, it's quite simple.

1. SBE Schema
-------------

This NuGet package contains a sample schema in \sample\TestSchema.xml (all files referenced in this readme file can be found in \packages\Adaptive.SBE.{version}\ folder contained in your solution directory)

We will use this schema 'as is' in this example but you can use it later as a starting point to define your own schema.

2. SBE Tool
-----------

Once you have a schema you need to use SBE Tool to generate the C# encoders and decoders to marshal your messages.
This tool is located at the following path in the package: \tools\SBETools.jar

It is a Java program so you will need a recent version of Java installed on your PC and have java in your PATH environment variable:
 - download and install java: http://www.java.com/en/download/help/download_options.xml#windows
 - set java in your PATH: http://www.java.com/en/download/help/path.xml

Note: we will likely implement the code generation tool at some point in .NET

Once you have java installed and working you can use SBETool to generate the C# code with SBE Tool:
the package contains the file \sample\GenerateSampleCode.bat that you need to launch to generate the sample code.

Note that this bat file assumes that SBETool and the schema are still located at the same place in the package, if you have moved them around you will need to edit the .bat file.

The C# files should now have been generated, you will find them in \sample\output\

3. Generated code
-----------------

You will now need to add the generated code to a new project or use the current project (if you have added this NuGet package to an existing project).

Move all the generated files (\sample\output\*.cs) to the root of the project.

You will also need to add \lib\net40\sbe.dll as a reference to your project, if it's not already there (if you added the NuGet package to an existing project it should already be there).

The package also contains a sample class which manipulates the generated code to encode a message and then decode it (\sample\ExampleUsingGeneratedStub.cs)
Add this file to your project and configure it as an entry point for the project (Project properties > Application > Startup object) 
and also change the project type to console application if it's not already one.

You should also go through this code to get a better idea how to use the generated code API for encoding and decoding.

Next steps
----------

This was just a quick sample but hopefully now you have an idea of the overall process you have to go through to use SBE.

To get all the details please refer to our wiki (https://github.com/real-logic/simple-binary-encoding/wiki/) and especially the following sections:
 - schema definition: https://github.com/real-logic/simple-binary-encoding/wiki/FIX-SBE-XML-Primer
 - SBE Tool command line: https://github.com/real-logic/simple-binary-encoding/wiki/Sbe-Tool-Guide

If you face an issue, a suggestion or a question, please use the issue tracker on the main GitHub repo: https://github.com/real-logic/simple-binary-encoding/issues?state=open

