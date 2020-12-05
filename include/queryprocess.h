#pragma once
#include "tree.h"
#include <cctype>
#include <fstream>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
class QueryProcess {
private:
	std::ifstream queryFile;
	inline static const char* defaultQueryFileName = "queries.txt";
	std::pair<std::string, std::string> splitLine(std::string_view line) {
		auto firstSplitPoint = line.find_first_of(" "),
			lastSplitPoint = line.find_first_of(" ", firstSplitPoint + 1);
		return std::make_pair(line.substr(lastSplitPoint + 1).data(), std::string(line.data() + firstSplitPoint + 1, line.data() + lastSplitPoint));
	}
	std::string processLine(const Tree& tree, std::string_view line) {
		if (line.starts_with("whatis")) {
			return tree.printSubComponentInfo(line.substr(line.find_first_of(" ") + 1));
		}
		if (line.starts_with("howmany")) {
			const auto& [parentName, childName] = splitLine(line);
			auto parentInfo = tree.findComponent(parentName);
			auto firstLetterUppercasedParentName = parentName;
			firstLetterUppercasedParentName[0] = std::toupper(firstLetterUppercasedParentName[0]);
			if (!parentInfo) {
				return "";
			}
			for (const auto& [hasComponentCount, subComponent] : (*parentInfo).get()->next) {
				if (subComponent->componentName == childName) {
					return firstLetterUppercasedParentName + " has " + std::to_string(hasComponentCount) + ' ' +
						childName + '\n';
				}
			}
			return firstLetterUppercasedParentName + " has 0 " + childName + '\n';
		}
		throw std::invalid_argument("query file format error!");
	}
	std::string printComponentCount(std::weak_ptr<TreeNode> tree,
		std::string_view parentComponentName,
		std::string_view childComponentName) {
		auto treePointer = tree.lock();
		if (treePointer == nullptr) {
			throw std::runtime_error("tree was removed before this operation!");
		}
		std::string result;
		for (const auto& [componentCount, subComponent] : treePointer->next) {
			if (subComponent->componentName == childComponentName) {
				result = std::string(childComponentName.data()) + " has" +
					std::to_string(componentCount) + ' ' +
					std::string(parentComponentName);
				return result;
			}
		}
	}

public:
	QueryProcess(std::string_view queryFileName = defaultQueryFileName)
		: queryFile(queryFileName.data()) {
		if (!queryFile) {
			throw std::runtime_error("open" + std::string(queryFileName.data()) +
				"failed.");
		}
	}
	std::string getQueryResult(const Tree& tree) {
		std::string queryFileLine, result;
		while (std::getline(queryFile, queryFileLine)) {
			result += processLine(tree, queryFileLine);
		}
		return result;
	}
};