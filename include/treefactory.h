#pragma once
#include "tree.h"
#include <array>
#include <charconv>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
class TreeFactory {
private:
  std::ifstream definitionFile;
  //默认文件名
  inline static const char *defaultDefinitionFileName = "definitions.txt";
  //处理每行数据的函数
  std::tuple<std::string, size_t, std::string>
  splitLine(std::string_view line) {
    auto firstSplitPoint = line.find_first_of(" "),
         lastSplitPoint = line.find_last_of(" ");
    //转换数字
    size_t hasComponentSize;
    auto idealEndNumberConvertPostion = line.data() + lastSplitPoint;
    if (auto status =
            std::from_chars(line.data() + firstSplitPoint + 1,
                            idealEndNumberConvertPostion, hasComponentSize);
        status.ec != std::errc{} ||
        status.ptr != idealEndNumberConvertPostion) {
      throw std::invalid_argument("definition file format error!");
    }
    //转换数字结束
    //构造返回值
    return std::make_tuple(
        std::string(line.data(), line.data() + firstSplitPoint),
        hasComponentSize, std::string(line.data() + lastSplitPoint + 1));
  }

public:
  TreeFactory(std::string_view definitionFileName = defaultDefinitionFileName)
      : definitionFile(definitionFileName.data()) {
    //打不开文件抛异常
    if (!definitionFile) {
      throw std::runtime_error("open" + std::string(definitionFileName.data()) +
                               "failed.");
    }
  }
  Tree getBean() {
    Tree bean;
    std::string processDefinitionFileLine;
    //一次处理一行数据
    while (std::getline(definitionFile, processDefinitionFileLine)) {
      const auto &[parentComponent, hasComponentSize, childComponent] =
          splitLine(processDefinitionFileLine);
      //向树添加元素
      bean.addComponent(parentComponent, childComponent, hasComponentSize);
    }
    return bean;
  }
};