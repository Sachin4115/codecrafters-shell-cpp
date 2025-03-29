#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
using namespace std;

enum CommandType{
  Builtin,
  Executable,
  Nonexistent,
};

struct FullCommandType{
  CommandType type;
  string executable_path;
};

vector<string> parse_command_to_string_vector(string command);
FullCommandType command_to_full_command_type(string command);
string find_command_executable_path(string command);
string find_command_in_path(string command, string path);

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  while(true){
    cout << "$ ";

    string input;
    getline(std::cin, input);

    vector<string> command_vector = parse_command_to_string_vector(input);

    if(command_vector.size()==0) continue;

    FullCommandType fct = command_to_full_command_type(input);

    if(fct.type == Builtin){
      if(command_vector[0]=="exit"){
        int exit_code = stoi(command_vector[1]);
        return exit_code;
      }
      if(command_vector[0]=="echo"){
        for(int i=1;i<command_vector.size();i++){
          if(i!=1) cout<<" ";
          cout<<command_vector[i];
        }
        cout<<endl;
        continue;
      }
      if(command_vector[0]=="type"){
        if(command_vector.size()<2) continue;
        string command_name = command_vector[1];
        FullCommandType command_type = command_to_full_command_type(command_name);

        switch (command_type.type){
          case Builtin:
            cout<< command_name << " is a shell builtin"<<endl;
            break;
          case Executable:
            cout<< command_name << " is " << command_type.executable_path << endl;
            break;
          case Nonexistent:
            cout<< command_name << " not found"<<endl;
            break;
          default:
            break;
        }
        continue;
      }
      continue;
    }
    if(fct.type == Executable){
      string command_with_full_path = fct.executable_path;
      for(int argument_number = 1; argument_number < command_vector.size(); argument_number++){
        command_with_full_path += " ";
        command_with_full_path += command_vector[argument_number];
      }
      const char* command = command_with_full_path.c_str();
      system(command);
      continue;
    }
    cout<< input << ": command not found"<<endl;
  }
}

vector<string> parse_command_to_string_vector(string command)
{
  vector<string> arguments;
  string complete_argument = "";

  for(char c:command){
    if(c==' '){
      arguments.push_back(complete_argument);
      complete_argument="";
    }else{
      complete_argument+=c;
    }
  }

  if(complete_argument!="")
    arguments.push_back(complete_argument);
  return arguments;
}

FullCommandType command_to_full_command_type(string command)
{
  vector<string> builtin_commands = {"exit","echo","type"};
  FullCommandType fct;
  if(find(builtin_commands.begin(),builtin_commands.end(),command)!=builtin_commands.end())
    fct.type = Builtin;

  string executable_path = find_command_executable_path(command);
  if(executable_path != ""){
    fct.type = Executable;
    fct.executable_path = executable_path;
  }else
    fct.executable_path = Nonexistent;
  return fct;
}

string find_command_executable_path(string command)
{
  string path_environment=getenv("PATH");
  if(path_environment=="") return "";
  string path_accumulate = "";
  for(char ch : path_environment){
    if(ch==':'){
      string executable_path = find_command_in_path(command,path_accumulate);
      if(executable_path!="") return executable_path;
      path_accumulate="";
    }
    else{
      path_accumulate += ch;
    }
  }
  string executable_path = find_command_in_path(command,path_accumulate);
  if(executable_path!="") return executable_path;
  return "";
}

string find_command_in_path(string command, string path)
{
  for(const auto &entry : filesystem::directory_iterator(path)){
    if(entry.path() == (path + "/" + command))
      return entry.path().string();
  }
  return "";
}


