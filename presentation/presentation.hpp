#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <list>
#include <vector>
#include <iostream>

#include "relator.hpp"
#include <utils/iterators.hpp>

class GroupPresentation {
private:
  using GenIter = std::unordered_map<size_t, std::string>::iterator;
  using RelIter = std::unordered_map<size_t, Relator>::iterator;
public:
  GroupPresentation(const std::vector<std::string> &generators,
        const std::vector<std::vector<int>> &relators) {
        for (size_t i = 0; i < generators.size(); ++i) {
          generators_[i + 1] = generators[i];
        }
        for (size_t i = 0; i < relators.size(); ++i) {
          relators_.emplace(std::make_pair(i, Relator(relators[i].begin(), relators[i].end())));
        }
      }

  size_t GeneratorCount() const { return generators_.size(); }

  std::string GetGeneratorByIndex(int index) const {
    if (index > 0) {
      return generators_.at(index);
    }
    return generators_.at(-index);
  }

  void RemoveGeneratorByIndex(int index) {
    if (index > 0) {
      generators_.erase(index);
    }
    generators_.erase(-index);
  }

  size_t RelatorCount() const { return relators_.size(); }

  IteratorRange<GenIter> Generators() { return IteratorRange<GenIter>(generators_.begin(), generators_.end()); }

  Relator &GetRelatorByIndex(size_t index) {
    return relators_.at(index);
  }

  void RemoveRelatorByIndex(size_t index) {
    relators_.erase(index);
  }

  IteratorRange<RelIter> Relators() { return IteratorRange<RelIter>(relators_.begin(), relators_.end()); }

  size_t RelatorsTotalLength() const {
    size_t len = 0;
    for (const auto &[_, relator] : relators_) {
      len += relator.Size();
    }
    return len;
  }

private:
  std::unordered_map<size_t, std::string> generators_;
  std::unordered_map<size_t, Relator> relators_;

  friend std::ostream &operator<<(std::ostream &out, GroupPresentation &presentation);  
};


std::ostream &operator<<(std::ostream &out, GroupPresentation &presentation) {
    out << '<';
    size_t i = 0;
    for (const auto &[_, gen] : presentation.generators_) {
      out << gen;
      if (++i < presentation.generators_.size()) {
        out << ", ";
      }
    }
    out << '|';
    i = 0;
    for (auto &[_, rel] : presentation.relators_) {
      for (int gen_idx : rel.Generators()) {
        out << presentation.GetGeneratorByIndex(gen_idx);
        if (gen_idx < 0) {
        out << '-';
        }
      }
      if (++i < presentation.relators_.size()) {
        out << ", ";
      }
    }
    out << ">";

    return out;
  }