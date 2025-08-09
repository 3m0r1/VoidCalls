import json, toml

from contextlib import contextmanager

class Typedef:
    identifiers: list[str]
    dependencies: list[str]
    definition: str

    added: bool # has it been added to file (meaning it wont be added later)?

    def __init__(self, jsonObj):
        self.identifiers = jsonObj['identifiers']
        self.dependencies = jsonObj['dependencies']
        self.definition = jsonObj['definition']
        self.added = False

class Argument:
    name: str
    type: str
    
    salIn: bool
    salOut: bool
    salOptional: bool

    def __init__(self, jsonObj):
        self.name = jsonObj['name']
        self.type = jsonObj['type']

        self.salIn = jsonObj['in']
        self.salOut = jsonObj['out']
        self.salOptional = jsonObj['optional']

class Prototype:
    name: str
    type: str
    arguments: list[Argument]

    def __init__(self, name: str, jsonObj):
        self.name = name
        self.type = jsonObj['type']
        self.arguments = []
        
        for arg in jsonObj['params']:
            self.arguments.append(
                Argument(arg)
            )    

    def getDependencies(self) -> list[str]:
        dependencies = [self.type]

        for arg in self.arguments:
            dependencies.append(arg.type)
        
        return dependencies

class DataReader:
    typedefs: dict[str, Typedef]
    prototypes: dict[str, Prototype]

    def __init__(self, typedefsPath: str, prototypesPath: str):
        self.typedefs = {}
        self.prototypes = {}

        with open(typedefsPath, 'r') as f:
            self.typedefsFile = json.load(f)
            
        with open(prototypesPath, 'r') as f:
            self.prototypesFile = json.load(f)

    def read(self):
        for typedefObj in self.typedefsFile:
            typedef = Typedef(typedefObj)
            self.typedefs[typedef.identifiers[0]] = typedef

        for name, prototypeObj in self.prototypesFile.items():
            prototype = Prototype(name, prototypeObj)
            self.prototypes[name] = prototype
    
    def getTypedef(self, id: str) -> Typedef:
        typedef = self.typedefs.get(id)

        if not typedef:
            for typedef in self.typedefs.values():
                if id in typedef.identifiers:
                    return typedef
                
            return None
        else:
            return typedef
        
    def getTypedefsRecursive(self, id: str) -> list[Typedef]:
        rootTypedef = self.getTypedef(id)

        if not rootTypedef:
            return []
        
        if rootTypedef.added:
            return []
        
        rootTypedef.added = True

        typedefs = []

        if len(rootTypedef.dependencies) <= 0:
            return [rootTypedef]
        
        for dependency in rootTypedef.dependencies:
            foundTypedefs = self.getTypedefsRecursive(dependency)

            typedefs.extend(foundTypedefs)

        typedefs.append(rootTypedef)

        return typedefs
        
class Metadata:
    name: str
    sal: bool

    def __init__(self, tomlObj: dict[str, any]):
        self.name = tomlObj.get('name', 'DefaultInstance')
        self.sal = tomlObj.get('sal', True)

SalTable = {
    0b00: (None, None),
    0b10: ('_In_', '_In_opt_'),
    0b01: ('_Out_', '_Out_opt_'),
    0b11: ('_Inout_', '_Inout_opt_')
}

class Invoker:
    metadata: Metadata
    module: str
    name: str
    alias: str
    type: int

    def __init__(self, module: str, name: str, tomlObj: dict[str, any], metadata: Metadata):
        self.module = module
        self.name = name
        self.alias = tomlObj.get('alias', name)
        self.type = tomlObj.get('type', 1)
        self.metadata = metadata

    def genArg(self, arg: Argument, indentation: int):
        if self.metadata.sal:
            mask = (arg.salIn << 1) | arg.salOut
            entry, entryOpt = SalTable[mask]

            return (' ' * (indentation * 4)) + f'{entryOpt if arg.salOptional else entry} {arg.type} {arg.name}'
        else:
            return (' ' * (indentation * 4)) + f'{arg.type} {arg.name}'
    
    def generate(self, prototype: Prototype, indentation: int):
        argList = ',\n'.join(self.genArg(arg, indentation) for arg in prototype.arguments)

        return f"{prototype.type} {self.alias}(\n{argList}\n{' ' * ( (indentation-1) * 4)})"

class ConfigReader:
    metadata: Metadata
    invokers: list[Invoker]

    def __init__(self, configPath: str):
        with open(configPath, 'r') as f:
            self.config = toml.loads(f.read())
    
    def read(self):
        self.metadata = Metadata(self.config['metadata'])
        self.invokers = []

        ntdllInvokers = self.config.get('ntdll')
        win32uInvokers = self.config.get('win32u')

        if ntdllInvokers:
            for name, invokerObj in ntdllInvokers.items():
                self.invokers.append(
                    Invoker('ntdll', name, invokerObj, self.metadata)
                )

        if win32uInvokers:
            for name, invokerObj in win32uInvokers.items():
                self.invokers.append(
                    Invoker('win32u', name, invokerObj, self.metadata)
                )


# TODO: write an abstract base class for Node
class CodeNode:
    prefix: str
    content: str
    terminator: str
    indentation: int

    def __init__(self, prefix: str, content: str, terminator: str = ';'):
        self.prefix = prefix
        self.content = content
        self.terminator = terminator
        self.indentation = 0
    
    def setIndentation(self, indentation: int):
        self.indentation = indentation

    def setParent(self, parent):
        self.parent = parent
        self.indentation = self.parent.indentation + 1

    def indentStr(self) -> str:
        return (' ' * (self.indentation * 4))
    
    def generate(self) -> str:
        parts = []
        if self.prefix:
            parts.append(self.prefix)
        if self.content:
            parts.append(self.content)
        line = ' '.join(parts)
        if self.terminator:
            line += self.terminator
        
        return self.indentStr() + line


class ContainerNode:
    keyword: str
    identifier: str
    start: str
    end: str
    indentation: int

    def __init__(self, keyword: str, identifier: str, start: str, end: str):
        self.keyword = keyword
        self.identifier = identifier
        self.start = start
        self.end = end
        self.indentation = 0
        self.children = []
    
    def setIndentation(self, indentation: int):
        self.indentation = indentation
    
    def setParent(self, parent):
        self.parent = parent
        self.indentation = self.parent.indentation + 1
    
    def add(self, child) -> 'ContainerNode':
        child.setParent(self)
        self.children.append(child)
        return self
    
    def addMultiple(self, children) -> 'ContainerNode':
        for child in children:
            self.add(child)
        return self
    
    def indentStr(self) -> str:
        return (' ' * (self.indentation * 4))
    
    def generate(self) -> str:
        headerParts = []
        if self.keyword:
            headerParts.append(self.keyword)
        if self.identifier:
            headerParts.append(self.identifier)
        headerLine = ' '.join(headerParts)
        if self.start:
            headerLine += f' {self.start}'

        childrenStr = '\n'.join(child.generate() for child in self.children)

        footerLine = ''

        if self.end:
            footerLine = self.indentStr() + self.end

        return self.indentStr() + headerLine + '\n' + childrenStr + '\n' + footerLine if footerLine else self.indentStr() + headerLine + '\n' + childrenStr


class StructsFile:
    headers: list[str] = ['windows.h'] # default headers

    def __init__(self, dataReader: DataReader, configReader: ConfigReader):
        self.dataReader = dataReader
        self.configReader = configReader
        self.file = open('./output/structs.h', 'w')

    def writeln(self, content: str):
        self.file.write(f'{content}\n')

    def generate(self):
        for header in self.headers:
            self.writeln(f'#include<{header}>')
        
        for invoker in self.configReader.invokers:
            prototype = self.dataReader.prototypes[invoker.name]
            if prototype:
                deps = prototype.getDependencies()
                for dep in deps:
                    typedefs = self.dataReader.getTypedefsRecursive(dep)
                    
                    for typedef in typedefs:
                        self.writeln(typedef.definition)

    def close(self):
        self.file.close()

class HeaderFile():
    headers: list[str] = ['structs.h']
    root: ContainerNode

    def __init__(self, dataReader: DataReader, configReader: ConfigReader):
        self.dataReader = dataReader
        self.configReader = configReader
        self.file = open('./output/instance.h', 'w')
        
        self.root = ContainerNode(
            keyword = 'class',
            identifier = self.configReader.metadata.name,
            start = '{',
            end = '};',
        )

    def writeln(self, content: str):
        self.file.write(f'{content}\n')

    def generate(self):
        self.writeln('#include "structs.h"')
        
        private = ContainerNode(
            keyword='private:',
            identifier = '',
            start = '',
            end = ''
        )

        self.root.add(private)

        private.add(
            CodeNode(
                prefix = 'Factory*',
                content = 'NtdllFactory',
                terminator = ';',
            )
        )

        for invoker in self.configReader.invokers:
            prototype = self.dataReader.prototypes[invoker.name]

            private.add(
                CodeNode(
                    prefix = f'Invoker<{prototype.type}>*',
                    content = f'{prototype.name}_Invoker',
                    terminator = ';',
                )
            )

        
        public = ContainerNode(
            keyword = 'public:',
            identifier = '',
            start = '',
            end = ''
        )

        self.root.add(public)

        fnNode = ContainerNode(
            keyword = 'Instance()',
            identifier = '',
            start = '{',
            end = '}'
        )

        public.add(fnNode)

        fnNode.add(
            CodeNode(
                prefix = '',
                content = 'this->NtdllFactory = new Factory(ModuleType::Ntdll, true)'
            )
        )

        for invoker in self.configReader.invokers:
            prototype = self.dataReader.prototypes[invoker.name]

            fnNode.add(
                CodeNode(
                    prefix = '',
                    content = f'this->{prototype.name}_Invoker = this->NtdllFactory->CreateInvoker<{prototype.type}>(hash::Djb2A("{prototype.name.replace('Nt', 'Zw')}"))'
                )
            )

            invokerFn = ContainerNode(
                keyword = '',
                identifier = invoker.generate(prototype, public.indentation + 2),
                start = '{',
                end = '}'
            )
            
            public.add(
                invokerFn
            )

            invokerFn.add(
                CodeNode(
                    prefix = '',
                    content = f'auto invoker = this->{prototype.name}_Invoker'
                )
            )

            callNode = ContainerNode(
                keyword = '',
                identifier = 'return invoker->InvokeIndirect' if invoker.type == 1 else 'return invoker->InvokeDirect',
                start = '(',
                end = ');'
            )

            invokerFn.add(callNode)

            for index, arg in enumerate(prototype.arguments):
                callNode.add(
                    CodeNode(
                        prefix = '',
                        content = arg.name,
                        terminator = ',' if len(prototype.arguments) - 1 > index  else ''
                    )
                )

        self.writeln(self.root.generate())

    def close(self):
        self.file.close()

class Generator:
    dataReader: DataReader
    configReader: ConfigReader

    structs: StructsFile
    header: HeaderFile

    def __init__(self, configPath: str):
        self.configReader = ConfigReader(configPath)
        
        self.dataReader = DataReader(
            './data/typedefs.json',
            './data/prototypes.json'
        )
        self.dataReader.read()
        self.configReader.read()

        self.structs = StructsFile(self.dataReader, self.configReader)
        self.header = HeaderFile(self.dataReader, self.configReader)

    def generate(self):
        self.structs.generate()
        self.structs.close()

        self.header.generate()
        self.header.close()

gen = Generator('config.toml')

gen.generate()
