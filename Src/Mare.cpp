
#include <nstd/Console.h>
#include <nstd/Error.h>
#include <nstd/Process.h>
#include <nstd/Variant.h>

#include "Mare.h"
#include "Make.h"

Mare::Mare(const String& configuration, const Make& make)
{
  for(List<Make::Target>::Iterator i = make.targets.begin(), end = make.targets.end(); i != end; ++i)
  {
    const Make::Target& makeTarget = *i;
    addTarget(makeTarget);
  }
}

void Mare::addTarget(const Make::Target& makeTarget)
{
  String extension = File::extension(makeTarget.outputFile);
  String targetName = File::basename(makeTarget.outputFile, extension);
  Target& target = targets.append(targetName, Target());

  HashSet<String> inputFiles = makeTarget.inputFiles;
  for(HashMap<String, Make::SourceFile>::Iterator i = makeTarget.files.begin(), end = makeTarget.files.end(); i != end; ++i)
  {
    const Make::SourceFile& sourceFile = *i;
    const String& objectFile = i.key();
    HashSet<String>::Iterator it = inputFiles.find(objectFile);
    if(it == inputFiles.end())
    {
      // todo: print warning
    }
    else
    {
      inputFiles.remove(it);
      Target::SourceFile& file = target.files.append(sourceFile.inputFile, Target::SourceFile());
      file.cppFlags = sourceFile.cppFlags;
      file.defines = sourceFile.defines;
      file.includePaths = sourceFile.includePaths;
      file.buildDir = File::dirname(objectFile);

      if(sourceFile.tool == "g++")
        file.type = "cppSource";
      else if(sourceFile.tool == "gcc")
        file.type = "cSource";
    }
  }
  target.additionalInputs.swap(inputFiles);

  // build global flag list
  HashSet<String> cppFlags;
  HashSet<String> includePaths;
  HashSet<String> defines;
  String buildDir;
  for(HashMap<String, Target::SourceFile>::Iterator i = target.files.begin(), end = target.files.end(); i != end;)
  {
    const Target::SourceFile& file = *i;
    cppFlags.append(file.cppFlags);
    includePaths.append(file.includePaths);
    defines.append(file.defines);

    if(buildDir.isEmpty())
      buildDir = file.buildDir;
    else if(file.buildDir != buildDir && buildDir != ".")
    {
      String fileBuildDir = File::dirname(file.buildDir);
      for(;;)
      {
        if(fileBuildDir.length() > buildDir.length() && fileBuildDir.startsWith(buildDir))
        {
          char lastChar = ((const char*)fileBuildDir)[buildDir.length()];
          if(lastChar == '/' || lastChar == '\\')
            break;
        }
        else if(fileBuildDir == buildDir)
          break;
        buildDir = File::dirname(buildDir);
        if(buildDir == ".")
          break;
      }
    }
  }
  if(buildDir == ".")
    buildDir.clear();

  for(HashMap<String, Target::SourceFile>::Iterator i = target.files.begin(), end = target.files.end(); i != end;)
  {
    const Target::SourceFile& file = *i;
    HashSet<String> diffCppFlags(cppFlags);
    HashSet<String> diffIncludePaths(includePaths);
    HashSet<String> diffDefines(defines);
    diffCppFlags.remove(file.cppFlags);
    diffIncludePaths.remove(file.includePaths);
    diffDefines.remove(file.defines);
    cppFlags.remove(diffCppFlags);
    includePaths.remove(diffIncludePaths);
    defines.remove(diffDefines);
  }
  for(HashMap<String, Target::SourceFile>::Iterator j = target.files.begin(), end = target.files.end(); j != end;)
  {
    Target::SourceFile& file = *j;
    file.cppFlags.remove(cppFlags);
    file.includePaths.remove(includePaths);
    file.defines.remove(defines);
    if(file.buildDir == buildDir)
      file.buildDir.clear();
    else if(file.buildDir.length() > buildDir.length() && file.buildDir.startsWith(buildDir))
    {
      char lastChar = ((const char*)file.buildDir)[buildDir.length()];
      if(lastChar == '/' || lastChar == '\\')
        file.buildDir = String("$(buildDir)") + file.buildDir.substr(buildDir.length());
    }
  }
  target.cppFlags.swap(cppFlags);
  target.includePaths.swap(includePaths);
  target.defines.swap(defines);
  target.buildDir = buildDir;
  target.outputDir = File::dirname(makeTarget.outputFile);
  if(!buildDir.isEmpty() && target.outputDir == target.buildDir)
    target.outputDir = String();

  // determine type
  if(makeTarget.tool == "gcc")
  {
    if(extension == "so")
      target.type = "cDynamicLibrary";
    else
      target.type = "cApplication";
  }
  else if(makeTarget.tool == "g++")
  {
    if(extension == "so")
      target.type = "cppDynamicLibrary";
    else
      target.type = "cppApplication";
  }
  else if(makeTarget.tool == "ar")
  {
    bool isC = true;
    for(HashMap<String, Target::SourceFile>::Iterator i = target.files.begin(), end = target.files.end(); i != end; ++i)
    {
      const Target::SourceFile& file = *i;
      if(file.type != "cSource")
      {
        isC = false;
        break;
      }
    }
    if(isC)
      target.type = "cStaticLibrary";
    else
      target.type = "cppStaticLibrary";
  }
}

void Mare::fileOpen(const String& path)
{
  if(!file.open(path))
  {
    Console::errorf("Could not open output file %s: %s\n", (const char*)path, (const char*)Error::getErrorString());
    Process::exit(1);
  }
}

void Mare::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    Console::errorf("Could not write to output file: %s\n", (const char*)Error::getErrorString());
    Process::exit(1);
  }
}

void Mare::fileWrite(const String& tabs, const Variant& data)
{
  const HashMap<String, Variant>& dataVar = data.toMap();
  for(HashMap<String, Variant>::Iterator i = dataVar.begin(), end = dataVar.end(); i != end; ++i)
  {
    const String& key = i.key();
    const Variant& var = *i;
    if(key == ".type")
      continue;
    if(var.getType() == Variant::mapType)
    {
      const HashMap<String, Variant>& dataVar = data.toMap();
      Variant& type = *dataVar.find(".type");
      if(!type.isNull())
        fileWrite(tabs + key + " = " + type.toString() + "{\n");
      else
        fileWrite(tabs + key + " = {\n");
      fileWrite(tabs + "  ", var);
      fileWrite(tabs + "}\n");
    }
    else if(var.getType() == Variant::listType)
    {
      const List<Variant>& listVar = var.toList();
      for(List<Variant>::Iterator i = listVar.begin(), end = listVar.end(); i != end; ++i)
        fileWrite(tabs + "\"" + i->toString() + "\"\n");
    }
    else
      fileWrite(tabs + key + " = \"" + var.toString() + "\"\n");
  }
}

void Mare::addHashSet(Variant& variant, const HashSet<String>& set)
{
  List<Variant>& listVar = variant.toList();
  for(HashSet<String>::Iterator i = set.begin(), end = set.end(); i != end; ++i)
    listVar.append(*i);
}

void Mare::generateMare(const String& outputFile)
{
  Variant data;
  HashMap<String, Variant>& targetsVar = data.toMap().append("targets", Variant()).toMap();
  for(const HashMap<String, Target>::Iterator i = targets.begin(), end = targets.end(); i != end; ++i)
  {
    const String& targetName = i.key();
    const Target& target = *i;
    HashMap<String, Variant>& targetVar = targetsVar.append(targetName, Variant()).toMap();

    if(!target.type.isEmpty())
      targetVar.append(".type", target.type);
    if(!target.cppFlags.isEmpty())
      addHashSet(targetVar.append("cppFlags", Variant()), target.cppFlags);
    if(!target.includePaths.isEmpty())
      addHashSet(targetVar.append("includePaths", Variant()), target.includePaths);
    if(!target.defines.isEmpty())
      addHashSet(targetVar.append("defines", Variant()), target.defines);
    if(!target.buildDir.isEmpty())
      targetVar.append("buildDir", target.buildDir);
    if(!target.outputDir.isEmpty())
      targetVar.append("outputDir", target.outputDir);
    if(!target.linkFlags.isEmpty())
      addHashSet(targetVar.append("linkFlags", Variant()), target.linkFlags);
    if(!target.libPaths.isEmpty())
      addHashSet(targetVar.append("libPaths", Variant()), target.libPaths);
    if(!target.libs.isEmpty())
      addHashSet(targetVar.append("libs", Variant()), target.libs);

    HashMap<String, Variant>& filesVar = targetVar.append("files", Variant()).toMap();
    for(HashMap<String, Target::SourceFile>::Iterator i = target.files.begin(), end = target.files.end(); i != end; ++i)
    {
      const String& inputFile = i.key();
      const Target::SourceFile& file = *i;
      if(!file.type.isEmpty())
        targetVar.append(".type", file.type);
      if(!file.cppFlags.isEmpty())
        addHashSet(targetVar.append("cppFlags", Variant()), target.cppFlags);
      if(!file.includePaths.isEmpty())
        addHashSet(targetVar.append("includePaths", Variant()), target.includePaths);
      if(!file.defines.isEmpty())
        addHashSet(targetVar.append("defines", Variant()), target.defines);
      if(!file.buildDir.isEmpty())
        targetVar.append("buildDir", file.buildDir);
    }
  }

  fileOpen(outputFile);
  fileWrite("", data);
}
