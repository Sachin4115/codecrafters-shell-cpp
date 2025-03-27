#include <iostream>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  while(true){
    cout << "$ ";
    string input;
    getline(std::cin, input);
    if(input=="exit 0") break;
    else if(input.substr(0,4)=="echo") cout<<input.substr(5)<<endl;
    else cout << input << ": command not found" <<endl;
  }
}