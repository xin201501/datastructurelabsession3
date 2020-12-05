#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

struct TreeNode {
  using MapType = std::unordered_multimap<size_t, std::shared_ptr<TreeNode>>;
  std::string componentName;
  size_t componentSize;
  MapType next;
  TreeNode() = default;
  TreeNode(std::string_view componentName, size_t componentSize = 0,
           const MapType &next = {})
      : componentName(componentName.data()), componentSize(componentSize),
        next(next) {}
  TreeNode(const TreeNode &another) = default;
  TreeNode(TreeNode &&another) noexcept
      : componentName(std::move(another.componentName)),
        componentSize(another.componentSize), next(std::move(another.next)) {}
  TreeNode &operator=(const TreeNode &another) {
    componentName = another.componentName;
    componentSize = another.componentSize;
    next = another.next;
    return *this;
  }
  TreeNode &operator=(TreeNode &&another) noexcept {
    std::exchange(*this, another);
    return *this;
  }
  std::string printSubComponentInfo() const {
    std::string result("Part " + std::string(componentName) +
                       " subparts are:\n");
    for (const auto &[hasComponentSize, child] : next) {
      result = result + std::to_string(hasComponentSize) + ' ' +
               child->componentName + '\n';
    }
    return result;
  }
  std::string visit() const {
    std::string result;
    result += printSubComponentInfo();
    for (const auto &[duumy, node] : next) {
      result += node->visit();
    }
    return result;
  }
};
class Tree {
public:
  using TreeNodePtr = TreeNode *;

private:
  std::shared_ptr<TreeNode> head;
  static void deepClone(TreeNodePtr &newInstance, const TreeNodePtr another) {
    if (another == nullptr) {
      return;
    }
    newInstance = new TreeNode(another->componentName, another->componentSize);
    for (const auto &[componentCount, component] : another->next) {
      auto insertPostion = newInstance->next.insert(
          {componentCount, std::make_shared<TreeNode>(*component)});
      auto nextCopyPostion = insertPostion->second.get();
      deepClone(nextCopyPostion, component.get());
    }
  }
  bool removeSubTree(std::shared_ptr<TreeNode> &beginLocation) {
    if (beginLocation == nullptr) {
      return false;
    }
    for (auto &[key, child] : beginLocation->next) {
      if (!removeSubTree(child)) {
        return false;
      }
    }
    beginLocation.reset();
    return true;
  }
  std::optional<std::reference_wrapper<std::shared_ptr<TreeNode>>>
  findComponent(std::string_view componentName,
                std::shared_ptr<TreeNode> &beginSearchLocation) {
    if (head == nullptr) {
      return std::nullopt;
    }
    if (beginSearchLocation->componentName == componentName) {
      return {std::reference_wrapper(beginSearchLocation)};
    }
    std::optional<std::reference_wrapper<std::shared_ptr<TreeNode>>> result;
    for (auto &[key, child] : beginSearchLocation->next) {
      result = findComponent(componentName, child);
      if (result.has_value()) {
        return {*result};
      }
    }
    return std::nullopt;
  }

public:
  Tree() : head(nullptr) {}
  //执行浅拷贝,多个通过拷贝构造而相关联的对象共享元素,若进行深拷贝请使用deepClone()
  Tree(const Tree &another) = default;
  static Tree deepClone(const Tree &another) {
    Tree instance;
    if (another.head == nullptr) {
      return instance;
    }
    auto nextCopyPostion = instance.head.get();
    deepClone(nextCopyPostion, another.head.get());
    instance.head.reset(nextCopyPostion);
    return instance;
  }

  Tree(Tree &&another) noexcept : head(another.head) { another.head.reset(); }
  decltype(auto) findComponent(std::string_view componentName) {
    return findComponent(componentName, head);
  }
  decltype(auto) findComponent(std::string_view componentName) const {
    return const_cast<Tree *>(this)->findComponent(componentName);
  }
  void addComponent(std::string_view parentName, std::string_view componentName,
                    size_t subComponentSize) {
    auto postion = findComponent(parentName);
    if (!postion) {
      if (head == nullptr) {
        head.reset(new TreeNode(parentName.data()));
        head->next.insert(
            {subComponentSize,
             std::make_shared<TreeNode>(componentName, subComponentSize)});
      }
      return;
    }
    (*postion).get()->next.insert(
        {subComponentSize,
         std::make_shared<TreeNode>(componentName, subComponentSize)});
  }
  //函数将节点和下方子树全部删除
  bool removeComponent(std::string_view componentName) {
    auto postion = findComponent(componentName);
    if (!postion) {
      return false;
    }
    return removeSubTree((*postion).get());
  }
  bool clear() { return removeSubTree(head); }
  template <typename... Args>
  void modifyComponentInfo(std::string_view componentName, Args... newInfo) {
    auto postion = findComponent(componentName);
    if (!postion) {
      return;
    }
    postion = std::make_shared<TreeNode>(std::forward<Args>(newInfo)...);
  }
  std::string printSubComponentInfo(std::string_view componentName) const {
    auto findPostion = findComponent(componentName);
    if (!findPostion) {
      return "";
    }
    return (*findPostion).get()->printSubComponentInfo();
  }
  std::string visit() const {
    if (head == nullptr) {
      return "";
    }
    return head->visit();
  }
};