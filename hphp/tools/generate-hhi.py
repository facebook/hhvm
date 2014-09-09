#!/usr/bin/env python3
'''
@copyright 2014, Miles Johnson
@link http://milesj.me
'''

'''
How to use:
- Clone the PHP docs SVN repository: https://svn.php.net/repository/phpdoc/modules/doc-en
- Execute the script from the command line `python3 /hphp/tools/generate-hhi.py --src=/path/to/svn/checkout`
- Pass `--type-hint` to the command to add Hack style type hinting to methods and properties

Known issues:
- No way to determine if a class is abstract
- hrtime
    - HRTime namespace has to manually be fixed
- xmldiff
    - XMLDiff namespace has to manually be fixed
- zip
    - ZipArchive constants are not defined in the class XML and are instead defined in constants.xml
'''

from argparse import ArgumentParser
import os
import re
import glob
import logging
import xml.etree.ElementTree as ET

parser = ArgumentParser(description='Generate .hhi files based on the PHP documentation.')
parser.add_argument('-s', '--src', dest='src', help='Path to the SVN checkout.')
parser.add_argument('-d', '--dest', dest='dest', default='../hack/hhi/stdlib/', help='Path to output the hhi files.')
parser.add_argument('-t', '--type-hint', dest='typeHint', help='Apply argument and return type hints.', action='store_true')

args = parser.parse_args()

# Define custom functions

def docblock_copyright():
    return '''
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
    '''.strip()

def type_hint(type, nullable = False):
    type = type.strip()

    if type == 'object' or type == 'ReturnType' or type == 'reference':
        type = 'mixed'

    elif type == 'char':
        type = 'string'

    elif type == 'integer' or type == 'number':
        type = 'int'

    elif type == 'double' or type == 'real':
        type = 'float'

    elif type == 'boolean':
        type = 'bool'

    elif type == 'Callable' or type == 'callback' or type == 'collable':
        type = 'callable' # typos :[

    elif type == 'resouce':
        type = 'resource'

    elif type == 'variant':
        type = 'Variant' # Looks like this is a class

    elif '|' in type:
        type = type.split('|')[0]

    if nullable and type != 'mixed':
        type = '?' + type

    return type

def function_name(name):
    if '::' in name:
        name = name.split('::')[1] # Some are written in OOP style

    name = name.replace('$', '')

    return name.lower()

def variable_name(name, is_ref = False):
    name = name.replace('-', '').replace('*', '').replace('"', '') # Some args have dashes and asterisks

    if name == '8bit':
        name = 'eightbit' # ncurses module

    if '|' in name:
        name = name.split('|')[0]

    if name[0] != '$':
        name = '$' + name

    if is_ref:
        name = '&' + name

    return name

def default_value(value):
    value = value.strip()

    if value == 'NULL':
        return 'null' # Return as lowercase

    elif value == '' or value == None or value == 'None':
        return '""' # Empty values

    elif value == '"BR /"':
        return '"<br/>"' # Fix broken HTML

    elif value == 'array()':
        return '[]' # Return as short hand

    elif value == 'int':
        return '-1' # Some defaults are the string 'int' for some reason

    elif ' | ' in value:
        return value.split(' | ')[0] # Bitmasked constants are not allowed

    elif re.match(r'^[a-z_]+\(', value):
        return '""' # Remove function calls

    elif re.match(r'[0-9]+', value) and '.' in value:
        return '"' + value + '"' # Wrap floats in strings

    return value

def clean_xml(xml):
    xml = xml.replace('&quot;', '"') # Convert HTML entities
    xml = xml.replace('&false;', 'false').replace('&true;', 'true').replace('&null;', 'null') # Convert types that are entities for some reason
    xml = xml.replace('xmlns="http://docbook.org/ns/docbook"', '') # Remove the XML namespace as it makes it complicated
    xml = re.sub(r"&([A-Za-z0-9\.\-_]+);", '', xml) # Remove invalid entities

    return xml

def parse_constants(hhi, xml):
    root = ET.fromstring(clean_xml(xml))
    constNodes = root.findall('variablelist/varlistentry')
    dupes = []

    # Some constants are in different XML locations
    if len(constNodes) <= 0:
        constNodes = root.findall('para/variablelist/varlistentry')

    for constNode in constNodes:
        name = constNode.find('term/constant').text

        if '::' in name:
            name = name.split('::')[1]

        if name in dupes:
            continue

        dupes.append(name)

        hhi.write("const {0} = '';\n".format(name))

def parse_function(hhi, xml, is_method = False):
    root = ET.fromstring(clean_xml(xml))
    descNode = root.find('refsect1[@role="description"]/methodsynopsis[last()]')

    # Some functions are aliases for other functions and contain no elements
    if descNode == None:
        return

    # Use the `refname` node as the `methodname` seems to be not 100% valid
    nameNode = root.find('refnamediv/refname')
    typeNode = descNode.find('type')
    modifierNode = descNode.find('modifier')

    # Gather the arguments
    arguments = []

    for argNode in descNode.findall('methodparam'):
        argNameNode = argNode.find('parameter')

        # Skip variadics
        if argNameNode.text == '...':
            arguments.append('...')
            continue

        argDefault = ''
        default = argNode.find('initializer')

        if default != None:
            if default.find('constant') != None:
                default = default.find('constant')

            argDefault = default_value(str(default.text))

        argName = variable_name(argNameNode.text, (argNameNode.get('role') == 'reference'))
        argType = type_hint(argNode.find('type').text, (argDefault == 'null'))

        # Format arguments
        if args.typeHint:
            arg = argType + ' ' + argName
        else:
            arg = argName

        if argDefault != '':
            arg += ' = ' + argDefault

        arguments.append( arg )

    # Format function
    if args.typeHint:
        funcFormat = 'function {0}({1}): {2}'
    else:
        funcFormat = 'function {0}({1})'

    name = str(nameNode.text)

    if typeNode == None:
        type = 'void'
    else:
        type = typeNode.text

    funcFormat = funcFormat.format(
        function_name(name),
        ', '.join(arguments),
        type_hint(type)
    )

    # Class methods
    if is_method:
        if modifierNode != None:
            funcFormat = modifierNode.text + ' ' + funcFormat
        else:
            funcFormat = 'public ' + funcFormat

        funcFormat = '    ' + funcFormat

    if 'abstract' in funcFormat:
        funcFormat = funcFormat + ';'
    else:
        funcFormat = funcFormat + ' {}'

    hhi.write(funcFormat + '\n')

def parse_class(hhi, xml, path):
    root = ET.fromstring(clean_xml(xml))
    descNode = root.find('partintro/section/classsynopsis')

    # Class name and inheritance
    namespace = ''
    className = ''
    extendName = ''
    interfaces = []

    for classNode in descNode.findall('classsynopsisinfo/ooclass'):
        classNameNode = classNode.find('classname')

        # Fixes the `streamWrapper` class
        if classNameNode.find('replaceable') != None:
            classNameNode = classNameNode.find('replaceable');

        if classNode.find('modifier') == None:
            className = classNameNode.text
        else:
            extendName = classNameNode.text

    # Some class names are found a parent up, so we must fix it (mongo for example)
    if className == '':
        className = root.find('partintro/section/classsynopsis/ooclass/classname').text

    if '\\' in className:
        classParts = className.split('\\')
        className = classParts.pop()
        namespace = '\\'.join(classParts)

    for interfaceNode in descNode.findall('classsynopsisinfo/oointerface/interfacename'):
        interfaces.append( interfaceNode.text )

    # Create the class name
    if namespace:
        classFormat = 'namespace ' + namespace + ' {\n'
    else:
        classFormat = ''

    # Look for the interface synopsis entity in the raw XML
    if 'interfacesynopsis' in xml:
        classFormat += 'interface ' + className
    else:
        classFormat += 'class ' + className

    if extendName != '' and extendName != className:
        classFormat += ' extends ' + extendName

    if len(interfaces) > 0:
        classFormat += ' implements ' + ', '.join(interfaces)

    hhi.write(classFormat + ' {\n')

    # Gather constants and properties
    for propNode in descNode.findall('fieldsynopsis'):
        build_property(hhi, propNode)

    # Gather methods
    for methodXmlPath in glob.glob(path + '/*.xml'):
        parse_function(hhi, open(methodXmlPath, 'r', encoding='utf-8').read(), True)

    hhi.write('}\n')

    if namespace:
        hhi.write('}\n')

def build_property(hhi, node):
    modifierNode = node.find('modifier')
    typeNode = node.find('type')
    defaultNode = node.find('initializer')
    name = node.find('varname').text
    default = ''

    if defaultNode != None:
        default = default_value(defaultNode.text)

    if modifierNode == None or modifierNode.text == 'readonly':
        propFormat = 'public'
    else:
        propFormat = modifierNode.text

    if args.typeHint and typeNode != None:
        propFormat += ' ' + type_hint(typeNode.text, (default == 'null'))

    if 'const' in propFormat:
        if '::' in name:
            name = name.split('::')[1]

        # Some constants string values aren't wrapped in quotes
        if default != '' and default[0] != '"' and typeNode != None and typeNode.text == 'string':
            default = '"' + re.sub('(^"|"$)', '', default) + '"'

        propFormat += ' ' + name
    else:
        propFormat += ' ' + variable_name(name)

    if default != '':
        propFormat += ' = ' + default
    elif 'const' in propFormat:
        propFormat += ' = ""';

    hhi.write('    ' + propFormat + ';\n')

# Determine valid paths

if not args.src:
    logging.error('A path to the SVN checkout is required')
    exit(1)

srcPath = os.path.abspath(args.src)
destPath = os.path.abspath(args.dest)
rootPath = os.path.abspath(srcPath + '/en/reference')

if not os.path.isdir(srcPath):
    logging.error('The source folder does not exist or is not a folder')
    exit(1)

elif not os.path.isdir(destPath):
    logging.error('The destination folder does not exist or is not a folder')
    exit(1)

elif not os.path.isdir(rootPath):
    logging.error('The /en/reference/ folder was not found in the SVN documentation')
    exit(1)

print('Generating hhi files...')
print('Source Path: ' + srcPath)
print('Destination Path: ' + destPath)
print('Documentation Path: ' + rootPath)
print('')

# Loop over the reference docs
for bookName in os.listdir(rootPath):
    bookPath = rootPath + '/' + bookName
    hhiPath = destPath + '/builtins_' + bookName + '.idl.hhi'
    writeCount = 0

    # Skip certain books
    if bookName in ['yaf']:
        continue

    print('--- ' + bookName + ' ---')

    # Create hhi file
    hhi = open(hhiPath, 'w+', encoding='utf-8')
    hhi.write('<?hh /* -*- php -*- */\n')
    hhi.write(docblock_copyright() + '\n')

    # Loop over constants
    if os.path.isfile(bookPath + '/constants.xml'):
        constXmlContents = open(bookPath + '/constants.xml', 'r', encoding='utf-8').read()

        parse_constants(hhi, constXmlContents)
        writeCount += 1

    # Loop over each XML file in the book functions directory
    if os.path.isdir(bookPath + '/functions'):
        for funcXmlPath in glob.glob(bookPath + '/functions/*.xml'):
            funcXmlName = os.path.basename(funcXmlPath)
            funcXmlContents = open(funcXmlPath, 'r', encoding='utf-8').read()

            parse_function(hhi, funcXmlContents)
            writeCount += 1

    # Loop over each XML file in the book directory
    for bookXmlPath in glob.glob(bookPath + '/*.xml'):
        bookXmlName = os.path.basename(bookXmlPath)

        # Skip over files that do not pertain to functions, classes, constants, etc
        if bookXmlName in ['book.xml', 'configure.xml', 'constants.xml', 'examples.xml', 'errors.xml', 'tables.xml',
            'ini.xml', 'reference.xml', 'setup.xml', 'versions.xml', 'appconfig.xml', 'tutorials.xml']:
            continue

        bookXmlContents = open(bookXmlPath, 'r', encoding='utf-8').read()

        # Skip over non class files as they are the only ones found in the root
        # All class XML files *should* contain `<ooclass>`
        if '<ooclass>' not in bookXmlContents:
            continue

        # Parse the XML document to generate the class
        parse_class(hhi, bookXmlContents, bookPath + '/' + bookXmlName.replace('.xml', ''))
        writeCount += 1

    # Close hhi file
    hhi.close()

    # Nothing was written to the file so remove it
    if writeCount <= 0:
        os.remove(hhiPath)

# Done!

print('')
print('hhi files successfully generated!')
exit(0)
