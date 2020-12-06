#include "queryprocess.h"
#include "tree.h"
#include "treefactory.h"
#include <iostream>
#include <vector>
// visual studio 编译器stdc++latest
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
int main() {
  system("chcp 65001");
  TreeFactory factory;
  Tree hospital = factory.getBean();
  QueryProcess process;
  cout << process.getQueryResult(hospital);
  cout << "---------------\n";
  hospital.visit([](const TreeNode &node) {
    cout << "component name:" << node.componentName
         << " size: " << node.hasComponentSize << '\n';
  });
  Tree hosptitalCopy = hospital.deepCopy();
  std::vector<TreeNode *> oldTreeNodeMemoryLocation, newTreeNodeMemoryLocation;
  hospital.visit([&oldTreeNodeMemoryLocation](TreeNode &node) mutable {
    oldTreeNodeMemoryLocation.push_back(&node);
  });
  hosptitalCopy.visit([&newTreeNodeMemoryLocation](TreeNode &node) mutable {
    newTreeNodeMemoryLocation.push_back(&node);
  });
  for (auto i1 = oldTreeNodeMemoryLocation.cbegin(),
            i2 = newTreeNodeMemoryLocation.cbegin();
       i1 != oldTreeNodeMemoryLocation.cend() &&
       i2 != newTreeNodeMemoryLocation.cend();
       i1++, i2++) {
    if (*i1 == *i2) {
      cerr << "this is shallow copy!" << endl;
      return -1;
    }
  }
  cout
      << "this is deep copy.validating if the contents of them are the same.\n";
  std::string sourceVisitResult, copyVisitResult;
  hospital.visit([&sourceVisitResult](const TreeNode &node) mutable {
    sourceVisitResult += node.printSubComponentInfo();
  });
  hosptitalCopy.visit([&copyVisitResult](const TreeNode &node) mutable {
    copyVisitResult += node.printSubComponentInfo();
  });
  if (sourceVisitResult != copyVisitResult) {
    cerr << "deep copy error!" << endl;
  } else {
    cout << "deep copy succeeded.\n";
  }
  hospital.clear();
  hosptitalCopy.clear();
  return 0;
}