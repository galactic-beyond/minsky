/*
  @copyright Steve Keen 2019
  @author Russell Standish
  This file is part of Minsky.

  Minsky is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minsky is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "minskyRS.h"
#include "RESTMinsky.h"
#include "minsky_epilogue.h"

using namespace classdesc;
using namespace std;

#ifdef READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

namespace minsky
{
  namespace
  {
    Minsky* l_minsky=NULL;
  }

  Minsky& minsky()
  {
    static Minsky s_minsky;
    if (l_minsky)
      return *l_minsky;
    return s_minsky;
  }

  LocalMinsky::LocalMinsky(Minsky& minsky) {l_minsky=&minsky;}
  LocalMinsky::~LocalMinsky() {l_minsky=NULL;}

  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool idleTasksOnly) {}
}

using namespace minsky;

RESTMinsky rminsky;

void processBuffer(const string& buffer)
{
  if (buffer[0]=='#') return;
  if (buffer[0]!='/')
    {
      cerr << buffer << "command doesn't starts with /"<<endl;
      return;
    }
  if (buffer=="/list")
    {
      for (auto& i: rminsky.registry)
        cout << i.first << endl;
      return;
    }

  try
    {
      auto n=buffer.find(' ');
      json_pack_t jin(json5_parser::mValue::null);
      string cmd;
      unsigned nargs=0;
      if (n==string::npos)
        cmd=buffer;
      else
        { // read argument(s)
          cmd=buffer.substr(0,n);
          read(buffer.substr(n),jin);
          nargs = jin.type()==json5_parser::array_type? jin.get_array().size(): 1;
        }
      cout<<cmd<<"=>";
      write(rminsky.registry.process(cmd, jin),cout);
      cout << endl;
      cmd.erase(0,1); // remove leading '/'
      replace(cmd.begin(), cmd.end(), '/', '.');
      rminsky.commandHook(cmd, nargs);
    }
  catch (const std::exception& ex)
    {
      cerr << "Exception: "<<ex.what() << endl;
    }
}

int main(int argc, const char* argv[])
{
  LocalMinsky lm(rminsky);
  RESTProcess(rminsky.registry,"/minsky",minsky::minsky());

  bool batch=argc>1 && argv[1]==string("-batch");
    
  
  char* c;
  string buffer;

#ifdef READLINE
  if (batch)
    while (getline(cin,buffer))
      processBuffer(buffer);
  else // interactive, use readline
    while ((c=readline("cmd>"))!=nullptr)
      {
        string buffer=c;
        processBuffer(buffer);
        if (strlen(c)) add_history(c); 
        free(c);
    }
#else
  while (getline(cin,buffer))
    processBuffer(buffer);
#endif
}
