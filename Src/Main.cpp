
#include <nstd/Console.h>
#include <nstd/String.h>
#include <nstd/HashMap.h>

#include "Make.h"
#include "Mare.h"

int_t main(int_t argc, char_t* argv[])
{
  // parse args
  HashMap<String, String> configurations;
  for(int i = 1; i < argc; ++i)
  {
    String arg;
    arg.attach(argv[i], String::length(argv[i]));
    if(arg.startsWith("-"))
    {
      // ./make2mare Release Debug:DEBUG=yess
      Console::printf("Usage: %s [ <config1>[:<make args>] [ <config2>[:<make args>] ... ] ]\n", argv[0]);
      return -1;
    }
    else
    {
      String configName = arg;
      String makeArgs;
      const char_t* colon = configName.find(':');
      if(colon)
      {
        size_t colonIndex = colon - (const char_t*)configName;
        configName = arg.substr(0, colonIndex);
        makeArgs = arg.substr(colonIndex + 1);
      }
      configurations.append(configName, makeArgs);
    }
  }
  if(configurations.isEmpty())
    configurations.append("Release", String());

  for(HashMap<String, String>::Iterator i = configurations.begin(), end = configurations.end(); i != end; ++i)
  {
    Make make;
    if(!make.load(*i))
      return -1;
    make.parse();
    Mare mare(i.key(), make);
    mare.generateMare("Marefile");
    break; // todo: support more than just the first configuration
  }

  return 0;
}
