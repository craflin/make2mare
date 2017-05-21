
#pragma once

#include <nstd/HashSet.h>
#include <nstd/HashMap.h>
#include <nstd/String.h>

#include "Make.h"

class Variant;

class Mare
{
public:
  Mare(const String& configuration, const Make& make);

  void generateMare(const String& outputFile);

private:
  class Configruation
  {
  public:
  };

  class Target
  {
  public:
    class SourceFile
    {
    public:
      String type;
      HashSet<String> cppFlags;
      HashSet<String> includePaths;
      HashSet<String> defines;
      String buildDir;
    };

  public:
    String type;

    HashSet<String> cppFlags;
    HashSet<String> includePaths;
    HashSet<String> defines;
    String buildDir;
    String outputDir;

    HashSet<String> linkFlags;
    HashSet<String> libPaths;
    HashSet<String> libs;

    HashMap<String, SourceFile> files;
    HashSet<String> additionalInputs;
  };

private:
  HashMap<String, Target> targets;
  File file;

private:
  void addTarget(const Make::Target& makeTarget);

  void fileOpen(const String& path);
  void fileWrite(const String& data);
  void fileWrite(const String& tabs, const Variant& data);

  void addHashSet(Variant& variant, const HashSet<String>& set);
};
