
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

  void_t generateMare(const String& outputFile);

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
  void_t addTarget(const Make::Target& makeTarget);

  void_t fileOpen(const String& path);
  void_t fileWrite(const String& data);
  void_t fileWrite(const String& tabs, const Variant& data);

  void_t addHashSet(Variant& variant, const HashSet<String>& set);
};
