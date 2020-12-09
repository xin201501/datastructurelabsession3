#include "queryprocess.h"
#include "tree.h"
#include "treefactory.h"
#include <iostream>
#include <thread>
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
    cout << "component name:" << node.componentName << " count: " << node.count
         << '\n';
  });
  Tree hospitalCopy = hospital.deepCopy();
  std::vector<TreeNode *> oldTreeNodeMemoryLocation, newTreeNodeMemoryLocation;
  std::thread pushOldHospitalMemoryIntoVectorThread([&]() {
    hospital.visit([&oldTreeNodeMemoryLocation](TreeNode &node) {
      oldTreeNodeMemoryLocation.push_back(&node);
    });
  });
  hospitalCopy.visit([&newTreeNodeMemoryLocation](TreeNode &node) {
    newTreeNodeMemoryLocation.push_back(&node);
  });
  pushOldHospitalMemoryIntoVectorThread.join();
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
  cout << "this is deep copy.validating if the contents of them are the "
          "same.\n";
  std::string sourceVisitResult, copyVisitResult;
  std::thread collectOldObjectInfoThread([&]() {
    hospital.visit([&sourceVisitResult](const TreeNode &node) {
      sourceVisitResult +=
          node.componentName + ' ' + std::to_string(node.count) + '\n';
    });
  });
  hospitalCopy.visit([&copyVisitResult](const TreeNode &node) {
    copyVisitResult +=
        node.componentName + ' ' + std::to_string(node.count) + '\n';
  });
  collectOldObjectInfoThread.join();
  if (sourceVisitResult != copyVisitResult) {
    cerr << "deep copy error! source visit result:\n"
         << sourceVisitResult << "-----------------\n"
         << "copy visit Result:\n"
         << copyVisitResult << endl;
  } else {
    cout << "deep copy succeeded.\n";
  }
  hospital.clear();
  hospitalCopy.clear();
  return 0;
}