#include <iostream>
#include <vector>
#include <iostream>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  vector<string> commands={"echo","exit","type"};
  while(true){
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
      if(b) cout<<input.substr(5)<<": not found"<<endl;
    }
    else cout << input << ": command not found" <<endl;
  }
}