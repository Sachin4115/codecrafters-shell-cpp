#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
using namespace std;

vector<string> commands={"echo","exit","type"};

string get_path(string command){
  // to get PATH environment variable
  string path_environment=getenv("PATH");
  // store path environment variable in string stream
  stringstream ss(path_environment);
  string path;
  while(!ss.eof()){
    // reads ss till : and stores in path
    getline(ss,path,':');
    // adds /command in the path
    string absolute_path = path + '/' + command;
    // checks if the path exists or not
    if(filesystem::exists(absolute_path)){
      return absolute_path;
    }
  }
  return "";
}

int main() {
  // Flush after every std::cout / std:cerr
  while(true){
    cout << unitbuf;
    cerr << unitbuf;
    cout << "$ ";
    string input;
    getline(std::cin, input);
    if(input=="exit 0") break;
    else if(input.substr(0,4)=="echo") cout<<input.substr(5)<<endl;
    else if(input.substr(0,4)=="type"){
      bool b=1;
      string match = input.substr(5);
      for(string command:commands){
        if(command==match){
          cout<<match<<" is a shell builtin"<<endl;
          b=0;
          break;
        }
      }
      if(b){
        string path = get_path(match);
        if(!path.empty()){
          cout<<match<<" is "<<path<<endl;
        }else{
          cout<<input.substr(5)<<" not found"<<endl;
        }
      }
    }
    else cout << input << ": command not found" <<endl;
  }
}