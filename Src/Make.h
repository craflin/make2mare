
#pragma once

#include <nstd/String.h>
#include <nstd/HashSet.h>
#include <nstd/HashMap.h>
#include <nstd/List.h>
#include <nstd/File.h>

class Make
{
public:
  class SourceFile
  {
  public:
    String tool;
    String inputFile;
    HashSet<String> cppFlags;
    HashSet<String> includePaths;
    HashSet<String> defines;
  };

  class Target
  {
  public:
    String tool;
    String outputFile;
    HashSet<String> linkFlags;
    HashSet<String> libPaths;
    HashSet<String> libs;
    HashSet<String> inputFiles;
    HashMap<String, SourceFile> files;
  };

public:
  List<Target> targets;

public:
  bool load(const String& makeArgs);
  void parse();

  /*

  void processData()
  {
    for(HashMap<String, Target>::Iterator i = targets.begin(), end = targets.end(); i != end; ++i)
    {
      Target& target = *i;
      processData(target);
    }
  }

  void processData(Target& target)
  {
    // removed object files that are not used as input files
    for(HashMap<String, SourceFile>::Iterator i = target.files.begin(), end = target.files.end(); i != end;)
    {
      const String& objectFile = i.key();
      HashSet<String>::Iterator it = target.inputFiles.find(objectFile);
      if(it == target.inputFiles.end())
      {
        // todo: print warning
        i = target.files.remove(i);
      }
      else
      {
        ++i;
        target.inputFiles.remove(it);
      }
    }

    // build global flag list
    HashSet<String> cppFlags;
    HashSet<String> includePaths;
    HashSet<String> defines;
    for(HashMap<String, SourceFile>::Iterator i = target.files.begin(), end = target.files.end(); i != end;)
    {
      const SourceFile& sourceFile = *i;
      cppFlags.append(sourceFile.cppFlags);
      includePaths.append(sourceFile.includePaths);
      defines.append(sourceFile.defines);
    }

    for(HashMap<String, SourceFile>::Iterator j = target.files.begin(), end = target.files.end(); j != end;)
    {
      const SourceFile& sourceFile = *j;
      HashSet<String> diffCppFlags(cppFlags);
      HashSet<String> diffIncludePaths(includePaths);
      HashSet<String> diffDefines(defines);
      diffCppFlags.remove(sourceFile.cppFlags);
      diffIncludePaths.remove(sourceFile.includePaths);
      diffDefines.remove(sourceFile.defines);
      cppFlags.remove(diffCppFlags);
      includePaths.remove(diffIncludePaths);
      defines.remove(diffDefines);
    }
    for(HashMap<String, SourceFile>::Iterator j = target.files.begin(), end = target.files.end(); j != end;)
    {
      SourceFile& sourceFile = *j;
      sourceFile.cppFlags.remove(cppFlags);
      sourceFile.includePaths.remove(includePaths);
      sourceFile.defines.remove(defines);
    }
    target.cppFlags.swap(cppFlags);
    target.includePaths.swap(includePaths);
    target.defines.swap(defines);
  }

  void generateMare(Mare& mare, const String& outputFile)
  {
    fileOpen(outputFile);
    fileWrite("targets = {\n");

    fileWrite("}\n");
  }

  */

private:
  List<String> lines;

private:
  static String buildFilePath(const String& rootDir, const String& cwd, const String& inputPath);
  static void splitArgs(const String& command, HashSet<String>& args);
};
