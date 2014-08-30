
#include <nstd/Process.h>
#include <nstd/Buffer.h>

#include "Make.h"

bool_t Make::load(const String& makeArgs)
{
  Process process;
  if(!process.open(String("make -B -n -w ") + makeArgs, Process::stdoutStream))
    return false;
  Buffer buffer;
  buffer.resize(4098);
  size_t bufferSize = 0;
  size_t i;
  String line;
  while((i = process.read((byte_t*)buffer + bufferSize, buffer.size() - bufferSize)) > 0)
  {
    bufferSize += i;
    for(;;)
    {
      const char_t* start = (const char_t*)(const byte_t*)buffer;
      const char_t* end = String::find(start, '\n');
      if(!end)
      {
        if(bufferSize == buffer.size())
          buffer.resize(buffer.size() * 2);
        break;
      }
      *(char_t*)end = '\0';
      size_t len = end - start;
      line.attach(start, len);
      lines.append(line);
      ++len;
      buffer.removeFront(len);
      buffer.resize(buffer.capacity());
    }
  }
  return true;
}

void_t Make::parse()
{
  HashMap<String, SourceFile> files;
  HashSet<String> args;
  String cwd;
  List<String> cwdStack;
  String rootDir;
  for(List<String>::Iterator i = lines.begin(), end = lines.end(); i != end; ++i)
  {
    const String& line = *i;
    splitArgs(line, args);
    if(args.isEmpty())
      continue;
    const String& tool = args.front();
    if(tool.startsWith("make[") || tool.startsWith("make:"))
    {
      const char_t* dir = tool.find("Entering directory");
      if(dir)
      {
        dir += 18;
        while(String::find(" '\"`", *dir))
          ++dir;
        const char_t* end = String::findOneOf(dir, dir[-1] == ' ' ? " '\"`" : "'\"`");
        cwd = File::simplifyPath(String(dir, end ? end - dir : String::length(dir)));
        cwdStack.append(cwd);
        if(rootDir.isEmpty())
          rootDir = cwd;
      }
      else if(tool.find("Leaving directory"))
      {
        cwdStack.removeBack();
        cwd = cwdStack.isEmpty() ? String() : cwdStack.back();
      }
      else
      {
        // todo: print warning
      }
    }
    else if(tool == "g++" || tool == "gcc")
    {
      if(args.find("-c") != args.end()) // source file
      {
        HashSet<String> cppFlags;
        HashSet<String> includePaths;
        HashSet<String> defines;
        String outputFile;
        String inputFile;
        for(HashSet<String>::Iterator i = ++args.begin(), end = args.end(); i != end; ++i)
        {
          const String& arg = *i;
          if(arg == "-o") // output file
          {
            HashSet<String>::Iterator next = i; ++next;
            if(next != end)
            {
              outputFile = buildFilePath(rootDir, cwd, *next);
              i = next;
            }
          }
          else if(arg.startsWith("-I")) // include path
            includePaths.append(buildFilePath(rootDir, cwd, arg.substr(2)));
          else if(arg.startsWith("-D")) // define
            defines.append(arg.substr(2));
          else if(arg.startsWith("-")) // cpp flag
            cppFlags.append(arg);
          else
            inputFile = arg;
        }
        if(!outputFile.isEmpty())
        {
          SourceFile& sourceFile = files.append(outputFile, SourceFile());
          sourceFile.tool = tool;
          sourceFile.inputFile = inputFile;
          sourceFile.cppFlags.swap(cppFlags);
          sourceFile.includePaths.swap(includePaths);
          sourceFile.defines.swap(defines);
        }
        else
        {
          // todo: print warnig
        }
      }
      else // target
      {
        HashSet<String> linkFlags;
        HashSet<String> libPaths;
        HashSet<String> libs;
        String outputFile;
        HashSet<String> inputFiles;
        for(HashSet<String>::Iterator i = ++args.begin(), end = args.end(); i != end; ++i)
        {
          const String& arg = *i;
          if(arg == "-o") // output file
          {
            HashSet<String>::Iterator next = i; ++next;
            if(next != end)
            {
              outputFile = buildFilePath(rootDir, cwd, *next);
              i = next;
            }
          }
          else if(arg.startsWith("-L")) // lib path
            libPaths.append(buildFilePath(rootDir, cwd, arg.substr(2)));
          else if(arg.startsWith("-l")) // lib
            libs.append(arg.substr(2));
          else if(arg.startsWith("-")) // cpp flag
            linkFlags.append(arg);
          else
            inputFiles.append(arg);
        }
        if(!outputFile.isEmpty())
        {
          Target& target = targets.append(Target());
          target.tool = tool;
          target.outputFile = outputFile,
          target.linkFlags.swap(linkFlags);
          target.libPaths.swap(libPaths);
          target.libs.swap(libs);
          target.inputFiles.swap(inputFiles);
          target.files.swap(files);
        }
        else
        {
          // todo: print warnig
        }
      }
    }
    else if(tool == "echo" || tool == "mkdir")
    {
      // ignore
    }
    else
    {
      // todo: print warnig
    }
  }
}

String Make::buildFilePath(const String& rootDir, const String& cwd, const String& inputPath)
{
  String path = File::isAbsolutePath(inputPath) ? inputPath : File::simplifyPath(cwd + "/" + inputPath);
  String relPath = File::getRelativePath(rootDir, path);
  return relPath.startsWith("..") ? path : relPath;
}

void_t Make::splitArgs(const String& command, HashSet<String>& args)
{
  args.clear();
  const char_t* str = command;
  for(;;)
  {
    while(String::isSpace(*str))
      ++str;
    String arg;
    if(*str == '"')
    {
    yeahQuote:
      ++str;
      while(*str)
      {
        if(*str == '"')
          break;
        else if(*str == '\\')
        {
          if(str[1] == '\\' || str[1] == '"')
          {
            arg.append(str[1]);
            str += 2;
          }
          else
          {
            arg.append(*str);
            ++str;
          }
        }
        else
        {
          arg.append(*str);
          ++str;
        }
      }
      args.append(arg);
    }
    else
    {
      while(*str && !String::isSpace(*str))
      {
        if(*str == '"')
          goto yeahQuote;
        else
        {
          arg.append(*str);
          ++str;
        }
      }
      args.append(arg);
    }
  }
}

