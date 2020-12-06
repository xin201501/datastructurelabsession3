#pragma once
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
  size_t hasComponentSize;
  MapType next;
  TreeNode() = default;
  TreeNode(std::string_view componentName, size_t hasComponentSize = 0,
           const MapType &next = {})
      : componentName(componentName.data()), hasComponentSize(hasComponentSize),
        next(next) {}
  TreeNode(const TreeNode &another) = default;
  TreeNode(TreeNode &&another) noexcept
      : componentName(std::move(another.componentName)),
        hasComponentSize(another.hasComponentSize),
        next(std::move(another.next)) {}
  TreeNode &operator=(const TreeNode &another) {
    componentName = another.componentName;
    hasComponentSize = another.hasComponentSize;
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
  template <typename T>
  T visit(const std::function<T(const TreeNode &)> &visitWay) const {
    return const_cast<TreeNode *>(this)->visit(visitWay);
  }
  template <typename T> T visit(const std::function<T(TreeNode &)> &visitWay) {
    T result;
    for (const auto &[dummy, child] : next) {
      result += visitWay(child);
    }
    return result;
  }
  void visit(const std::function<void(TreeNode &)> &visitWay) {
    for (const auto &[dummy, child] : next) {
      visitWay(*child);
    }
  }
};
class Tree {
public:
  using TreeNodePtr = TreeNode *;

private:
  std::shared_ptr<TreeNode> head;
  void deepCopy(TreeNodePtr &newInstance, const TreeNodePtr another) {
    if (another == nullptr) {
      return;
    }
    newInstance =
        new TreeNode(another->componentName, another->hasComponentSize);
    for (const auto &[hasComponentSize, component] : another->next) {
      auto insertPostion = newInstance->next.insert(
          {hasComponentSize,
           std::make_shared<TreeNode>(component->componentName,
                                      component->hasComponentSize)});
      auto nextCopyPostion = insertPostion->second.get();
      deepCopy(nextCopyPostion, component.get());
      insertPostion->second.reset(nextCopyPostion);
    }
  }
  bool removeSubTree(std::shared_ptr<TreeNode> &beginLocation) {
    if (beginLocation == nullptr) {
      return false;
    }
    for (auto &[dummy, child] : beginLocation->next) {
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
    for (auto &[hasComponentSize, child] : beginSearchLocation->next) {
      result = findComponent(componentName, child);
      if (result.has_value()) {
        return {*result};
      }
    }
    return std::nullopt;
  }
  template <typename T>
  T visit(const std::function<T(const TreeNode &)> &visitWay,
          TreeNodePtr &position) {
    T result;
    if (position == nullptr) {
      return result;
    }
    result += visitWay(*position);
    for (auto &&[dummy, nextNodes] : position->next) {
      auto nextVisitPtr = nextNodes.get();
      result += visit(visitWay, nextVisitPtr);
    }
    return result;
  }
  void visit(const std::function<void(TreeNode &)> &visitWay,
             TreeNodePtr &position) {
    if (position == nullptr) {
      return;
    }
    visitWay(*position);
    for (auto &&[dummy, nextNodes] : position->next) {
      auto nextVisitPtr = nextNodes.get();
      visit(visitWay, nextVisitPtr);
    }
  }

public:
  Tree() : head(nullptr) {}
  //执行浅拷贝,多个通过拷贝构造而相关联的对象共享元素,若进行深拷贝请使用deepCopy()
  Tree(const Tree &another) = default;
  Tree deepCopy() {
    Tree instance;
    if (head == nullptr) {
      return instance;
    }
    auto nextCopyPostion = instance.head.get();
    deepCopy(nextCopyPostion, head.get());
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
    auto &postion = findComponent(componentName);
    if (!postion) {
      return;
    }
    auto &modifyPostion = *postion.get();
    auto oldComponentCount = postion->get().hasComponentSize;
    modifyPostion = std::make_shared<TreeNode>(std::forward<Args>(newInfo)...);
    auto newComponentCount = modifyPostion->hasComponentSize;
    if (oldComponentCount != newComponentCount) {
      modifyPostion->next.insert({newComponentCount, std::move(modifyPostion)});
      modifyPostion->next.erase(oldComponentCount);
    }
  }
  std::string printSubComponentInfo(std::string_view componentName) const {
    auto findPostion = findComponent(componentName);
    if (!findPostion) {
      return "";
    }
    return (*findPostion).get()->printSubComponentInfo();
  }
  template <typename T>
  T visit(const std::function<T(const TreeNode &)> &visitWay) const {
    return const_cast<TreeNode *>(this)->visit(visitWay);
  }
  template <typename T>
  T visit(const std::function<T(const TreeNode &)> &visitWay) {
    TreeNodePtr headPointer = head.get();
    return visit(visitWay, headPointer);
  }
  void visit(const std::function<void(TreeNode &)> &visitWay) {
    if (head == nullptr) {
      return;
    }
    TreeNode *headPointer = head.get();
    visit(visitWay, headPointer);
  }
};