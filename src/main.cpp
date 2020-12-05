#include "queryprocess.h"
#include "tree.h"
#include "treefactory.h"
#include <iostream>
// visual studio 编译器stdc++latest
using std::cin;
using std::cout;
using std::endl;
int main() {
  system("chcp 65001");
  TreeFactory factory;
  Tree hospital = factory.getBean();
  QueryProcess process;
  cout << process.getQueryResult(hospital) << "---------------\n";
  cout << hospital.visit();
  Tree s2 = Tree::deepClone(hospital);
  hospital.clear();
  return 0;
}