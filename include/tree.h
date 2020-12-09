#pragma once
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
struct TreeNode {
  using MapType = std::multimap<size_t, std::shared_ptr<TreeNode>>;
  std::string componentName;
  size_t count;
  MapType next;
  TreeNode() = default;
  TreeNode(std::string_view componentName, size_t count = 0,
           const MapType &next = {})
      : componentName(componentName.data()), count(count), next(next) {}
  TreeNode(const TreeNode &another) = default;
  TreeNode(TreeNode &&another) noexcept
      : componentName(std::move(another.componentName)), count(another.count),
        next(std::move(another.next)) {}
  TreeNode &operator=(const TreeNode &another) {
    componentName = another.componentName;
    count = another.count;
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
    for (const auto &[count, child] : next) {
      result =
          result + std::to_string(count) + ' ' + child->componentName + '\n';
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
    newInstance = new TreeNode(another->componentName, another->count);
    for (const auto &[count, component] : another->next) {
      auto insertposition = newInstance->next.insert(
          {count, std::make_shared<TreeNode>(component->componentName,
                                             component->count)});
      auto nextCopyposition = insertposition->second.get();
      deepCopy(nextCopyposition, component.get());
      insertposition->second.reset(nextCopyposition);
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
    for (auto &[count, child] : beginSearchLocation->next) {
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
    auto nextCopyposition = instance.head.get();
    deepCopy(nextCopyposition, head.get());
    instance.head.reset(nextCopyposition);
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
    auto position = findComponent(parentName);
    if (!position) {
      if (head == nullptr) {
        head.reset(new TreeNode(parentName.data()));
        head->next.insert(
            {subComponentSize,
             std::make_shared<TreeNode>(componentName, subComponentSize)});
      }
      return;
    }
    position->get()->next.insert(
        {subComponentSize,
         std::make_shared<TreeNode>(componentName, subComponentSize)});
  }
  //函数将节点和下方子树全部删除
  bool removeComponent(std::string_view componentName) {
    auto position = findComponent(componentName);
    if (!position) {
      return false;
    }
    return removeSubTree(position->get());
  }
  bool clear() { return removeSubTree(head); }
  template <typename... Args>
  void modifyComponentInfo(std::string_view componentName, Args... newInfo) {
    auto position = findComponent(componentName);
    if (!position) {
      return;
    }
    auto &modifyposition = position->get();
    auto oldComponentCount = position->get()->count;
    modifyposition = std::make_shared<TreeNode>(std::forward<Args>(newInfo)...);
    auto newComponentCount = modifyposition->count;
    if (oldComponentCount != newComponentCount) {
      modifyposition->next.insert(
          {newComponentCount, std::move(modifyposition)});
      modifyposition->next.erase(oldComponentCount);
    }
  }
  std::string printSubComponentInfo(std::string_view componentName) const {
    auto findposition = findComponent(componentName);
    if (!findposition) {
      return "";
    }
    return (*findposition).get()->printSubComponentInfo();
  }
  template <typename T>
  T visit(const std::function<T(const TreeNode &)> &visitWay) const {
    return const_cast<Tree *>(this)->visit(visitWay);
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