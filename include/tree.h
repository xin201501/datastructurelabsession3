#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <vector>

struct TreeNode {
  std::string componentName;
  size_t componentSize;
  std::shared_ptr<std::vector<TreeNode *>> next;
  TreeNode() = default;
  TreeNode(std::string_view componentName, size_t componentSize,
           const std::vector<TreeNode *> &next = {})
      : componentName(componentName.data()), componentSize(componentSize),
        next(new std::vector(next)) {}
};
class Tree {
public:
  using TreeNodePtr = TreeNode *;

private:
  std::shared_ptr<TreeNode> head;
  bool addComponentProcess(std::string_view parentName,
                           std::string_view componentName, size_t componentSize,
                           TreeNodePtr beginSearchLocation) {
    if (beginSearchLocation->componentName == parentName) {
      beginSearchLocation->next->push_back(
          new TreeNode(componentName, componentSize));
      return true;
    }
    for (auto &&subComponent : *(beginSearchLocation->next)) {
      if (addComponentProcess(parentName, componentName, componentSize,
                              subComponent)) {
        break;
      }
    }
    return false;
  }
  bool removeComponentProcess(std::string_view componentName,
                              TreeNodePtr beginSearchLocation) {
    size_t subComponentSize = beginSearchLocation->next->size();
    for (size_t i = 0; i < subComponentSize; i++) {
      if ((*beginSearchLocation->next)[i]->componentName == componentName) {
        (*beginSearchLocation->next)[i] = nullptr;
        return true;
      }
      removeComponentProcess(componentName, (*beginSearchLocation->next)[i]);
    }
    return false;
  }

public:
  Tree() : head(std::make_shared<TreeNode>()) {}
  Tree(const Tree &another) = default;
  Tree(Tree &&another) noexcept : head(another.head) { head.reset(); }
  void addComponent(std::string_view parentName, std::string_view componentName,
                    size_t subComponentsize) {
    if (head == nullptr) {
      return;
    }
    addComponentProcess(head->componentName, componentName, subComponentsize,
                        head.get());
  }
  //函数将节点和下方子树全部删除
  bool removeComponent(std::string_view componentName) {
    if (head == nullptr) {
      return false;
    }
    return removeComponentProcess(componentName, head.get());
  }
};