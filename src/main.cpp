#include <iostream>
#include <unordered_set>
#include <set>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <ranges>
#include <termios.h>
using namespace std;

string WORKING_DIR = filesystem::current_path().string();
string path_environment=getenv("PATH");


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
char handleTabPress(string &input);
void readInputWithTab(string &input);
unordered_set<string> getExternalCommands();

unordered_set<string> commands;

int main() {
  // Flush after every cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  while(true){
    vector<string> builtin_commands = {"exit","echo","type"};
    commands =  getExternalCommands();
    commands.insert(builtin_commands.begin(),builtin_commands.end());

    cout << "$ ";

    string input;
    // getline(std::cin, input);
    readInputWithTab(input);

    vector<string> command_vector = parse_command_to_string_vector(input);

    if(command_vector.size()==0) continue;

    string output;
    string redirectFile;
    int flag = 0;
    int k=0;

    for(int i=1;i<command_vector.size();i++){
      if(command_vector[i] == ">" || command_vector[i] == "1>"){
        if(i+1<command_vector.size()){
          flag=1;
          redirectFile = command_vector[i+1];
          k=2;
          break;
        }
      }else if(command_vector[i] == "2>"){
        k=2;
        flag=2;
        redirectFile = command_vector[i+1];
        break;
      }else if(command_vector[i] == "1>>"){
        k=2;
        flag=3;
        redirectFile = command_vector[i+1];
        break;
      }else if(command_vector[i] == "2>>"){
        k=2;
        flag=4;
        redirectFile = command_vector[i+1];
        break;
      }
    }

    if(command_vector[0]=="cat"){
      system(input.c_str());
      continue;
    }else if(input.front()=='\'' || input.front()=='"'){
      char ch = input.front();
      string command_with_full_path = ch+command_vector[0]+ch;
      for(int argument_number = 1; argument_number < command_vector.size(); argument_number++){
        command_with_full_path += " ";
        command_with_full_path += command_vector[argument_number];
      }
      const char* command_ptr = command_with_full_path.c_str();
      system(command_ptr);
      continue;
    }


    FullCommandType fct = command_to_full_command_type(command_vector[0]);

    if(fct.type == Builtin){
      if(command_vector[0]=="exit"){
        int exit_code = stoi(command_vector[1]);
        return exit_code;
      }
      if(command_vector[0]=="echo"){
        for(int i=1;i<command_vector.size()-k;i++){
          if(i!=1) output+=" ";
          output+=command_vector[i];
        }
        output+="\n";
        if(flag==1){
          ofstream fileStream(redirectFile);
          fileStream<<output;
          fileStream.close();
        }else if(flag==2){
          ofstream fileStream(redirectFile);
          fileStream<<"";
          fileStream.close();
          cout<<output;
        }else if(flag==3){
          ofstream fileStream(redirectFile,ios_base::app);
          fileStream<<output;
          fileStream.close();
        }else if(flag==4){
          ofstream fileStream(redirectFile,ios_base::app);
          fileStream<<"";
          fileStream.close();
          cout<<output;
        }else {
          cout<<output;
        }
        continue;
      }
      if(command_vector[0]=="type"){
        if(command_vector.size()<2) continue;
        string command_name = command_vector[1];
        FullCommandType command_type = command_to_full_command_type(command_name);

        switch (command_type.type){
          case Builtin:
            cout<< command_name + " is a shell builtin" + "\n";
            break;
          case Executable:
            cout<< command_name + " is " + command_type.executable_path + "\n";
            break;
          case Nonexistent:
            cout<< command_name + " not found"+"\n";
            break;
          default:
            break;
        }
        continue;
      }
      if(command_vector[0]=="pwd"){
        cout<<WORKING_DIR + "\n";
      }
      if(command_vector[0]=="cd"){
        if(command_vector.size()!=2) cout<<"Syntax of command CD is incorrect";
        else{
          string arg = command_vector[1];
          string new_dir = WORKING_DIR;
          if(arg[0]=='/' && filesystem::exists(arg)){
            new_dir = arg;
            if(new_dir.size()==0) return 0;
            if(chdir(new_dir.c_str()) == -1) cout<<"cd: "<< arg <<": No such file or directory"<<endl;
          }else if(arg[0]=='.'){
            string cwd = filesystem::current_path().string();
            string dir = cwd + "/" + arg;
            new_dir = filesystem::canonical(dir).string();
            if(chdir(new_dir.c_str()) == -1) cout<<"cd: "<< arg <<": No such file or directory"<<endl;
          }else if(arg[0]=='~'){
            new_dir = getenv("HOME");
            chdir(new_dir.c_str());
            WORKING_DIR = new_dir;
          }
          else
            cout<< command_vector[1] <<": No such file or directory"<<endl;
          WORKING_DIR = new_dir;
        }
      }
      continue;
    }
    if(fct.type == Executable){
      string command_with_full_path = "";
      for(int argument_number = 0; argument_number < command_vector.size(); argument_number++){
        command_with_full_path += " ";
        command_with_full_path += command_vector[argument_number];
      }
      const char* command_ptr = command_with_full_path.c_str();
      system(command_ptr);
      continue;
    }
      cout<< input << ": command not found"<<endl;
  }
}

vector<string> parse_command_to_string_vector(string command)
{
  vector<string> arguments;
  string complete_argument = "";

  for(int i = 0; i<command.length();i++){
    char c = command[i];
    if(c=='\"'){
      i++;
      while(command[i]!='\"' || (i<command.length()-1 && command[i+1] == '\"') || (command[i-1]=='\"')){
        if(command[i]=='\\' && i<command.length()-1 ){
          if(command[i+1]=='\\' || command[i+1]=='\"'){
            complete_argument+=command[i+1];
            i++;
          }else{
            complete_argument+=command[i];
          }
        }
        else if(command[i]!='\"')
          complete_argument+=command[i];
        i++;
      }
      // arguments.push_back(complete_argument);
      // complete_argument="";
    }
    else if(c=='\''){
      i++;
      while(command[i]!='\'' || (i<command.length()-1 && command[i+1] == '\'') || (command[i-1]=='\'')){
        if(command[i]!='\'')
          complete_argument+=command[i];
        i++;
      }
      // arguments.push_back(complete_argument);
      // complete_argument="";
    }
    else if(c=='\\'){
      i++;
      complete_argument+=command[i];
    }else if(c==' '){
      if(complete_argument!="")
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
  vector<string> builtin_commands = {"exit","echo","type","pwd","cd"};
  FullCommandType fct;
  if(find(builtin_commands.begin(),builtin_commands.end(),command)!=builtin_commands.end()){
    fct.type = Builtin;
    return fct;
  }
  string executable_path = find_command_executable_path(command);
   if(executable_path != ""){
    fct.type = Executable;
    fct.executable_path = executable_path;
    return fct;
  }
  fct.type = Nonexistent;
  return fct;
}

string find_command_executable_path(string command)
{
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
    if(entry.path().string() == (path + "/" + command))
      return entry.path().string();
  }
  return "";
}

void enableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
void disableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

unordered_set<string> getExternalCommands()
{
  istringstream iss(path_environment);
  string dir;
  while (getline(iss, dir, ':')) {
    for (const auto& entry : filesystem::directory_iterator(dir)) {
      if (entry.is_regular_file()) {
          commands.insert(entry.path().filename().string());
      }
    }
  }
  return commands;
}

string find_common_prefix(const set<string>& matches) {
  if (matches.empty()) return "";
  if (matches.size() == 1) return *matches.begin();
  const string& first = *matches.begin();
  size_t min_len = first.length();
  for (const auto& str : matches) {
      min_len = min(min_len, str.length());
  }

  string result;
  for (size_t i = 0; i < min_len; i++) {
      char current = first[i];
      for (const auto& str : matches) {
          if (str[i] != current) {
              return result;
          }
      }
      result += current;
  }

  return result;
}

char handleTabPress(string &input,bool b)
{
  // auto matches = commands | views::filter([&input](const string& cmd) {
  //   return cmd.starts_with(input);
  // });
  set<string> matches;
  for(string c : commands){
    if(input== c.substr(0,input.length())){
      matches.insert(c);
    }
  }
  if (matches.empty()) {
    cout << '\a';
  }
  if (matches.size() == 1) {
    while (!input.empty()) {
      cout << "\b \b";
      input.pop_back();
    }
    input = *matches.begin() + " ";
    cout << input;
  }
  else if(!b){
    string common = find_common_prefix(matches);
    if(common.length()>input.length()){
      while (!input.empty()) {
        cout << "\b \b";
        input.pop_back();
      }
      input = common;
      cout << input;
    }
    else{
      cout << '\a';
      enableRawMode();
      char c;
      c = getchar();
      disableRawMode();
      if(c!='\t') return c;
      return handleTabPress(input,1);
    }
  }else{
    cout<<endl;
    for(const auto& match:matches){
      cout<<match<<"  ";
    }
    cout<<endl;
    cout<<"$ ";
    cout<<input;
  }
  return '$';
}


void helpReadInputWithTab(string &input,char c){
  if (c == '\n') {
    cout << endl;
    return;
  } else if (c == '\t') {
    char r = handleTabPress(input,0);
    if(r=='$') return;
    helpReadInputWithTab(input,r);
  } else if (c == 127) {
    if (!input.empty()) {
      input.pop_back();
      cout << "\b \b";
    }
  } else {
    input += c;
    cout << c;
  }
}

void readInputWithTab(string &input)
{
  enableRawMode();
  char c;
  while (true) {
    c = getchar();
    helpReadInputWithTab(input,c);
    if (c == '\n') {
      // cout << endl;
      return;
    }
  }
  disableRawMode();
}